"""
Created on Tue Jun 14 16:55:52 2016

@author: Gijsbert ter Horst

See: https://community.arm.com/groups/processors/blog/2013/09/25/branch-and-call-sequences-explained
for information on branching in ARM.
"""
from __future__ import print_function
import sys
import argparse
import fileinput
import pydot
import html
import inspect
import os

class InjectorError(Exception):
    pass

BYTEORDER='little'
#BYTEORDER='big'

def get_script_dir():
    filename = inspect.getframeinfo(inspect.currentframe()).filename
    return os.path.dirname(os.path.abspath(filename))

def get_icode(filename):
        with open(os.path.join(get_script_dir(), 'injectioncode', filename), 'r') as file:
            return file.read()
        return None

def to_bytes(integer):
    if not isinstance(integer, int):
        raise InjectorException('This method only accepts integers')
    return integer.to_bytes(4,byteorder=BYTEORDER, signed=False)

class CFGNode():
    """ Comparisons between nodes are only valid if they are constructed from
    a single dot file that depicts a valid CFG.
    """
    def __init__(self, node):
        # Underlying pydot.Node
        self.node = node
        # Unescape HTML codes, restore newlines, and remove some start and end brackets.
        node_label = html.unescape(node.get_label()).replace('\\l', '\n').replace(' | ', '\n')[2:-3].strip().split('\n')
        # The header for the basic block looks like:
        # <f0> 0x010654 (0x0105e8) main+0x6c
        bb_header = node_label[0].split()
        # All lines except the header.
        self.instructions = node_label[1:]
        # The address of the function is in brackets.
        self.func_addr   = int(bb_header[2][1:-2], 16)
        self.func_name   = bb_header[3].partition('+')[0]
        # The address of the block start.
        self.bb_start    = int(bb_header[1], 16)
        # The final line of the block will look like:
        # 0x10660:	pop	{fp, pc}
        self.bb_end      = int(self.instructions[-1].partition(':')[0], 16)
        # Address after code has been injected.
        self.patched_address = 0
        self.hotsiteid = None
        self.pre_nodes = []
        self.post_nodes = []

    def byte_repr(self):
        n = [to_bytes(self.hotsiteid)]
        pre_nodes = [to_bytes(node.patched_address) for node in self.pre_nodes]
        n.append(to_bytes(len(pre_nodes)))
        n.extend(pre_nodes)
        post_nodes = [to_bytes(node.patched_address) for node in self.post_nodes]
        n.append(to_bytes(len(post_nodes)))
        n.extend(post_nodes)
        return n
    # Enable sorting of nodes by start address
    # Assume no overlap (see class docstring)
    def __eq__(self, cfgnode):
        return self.bb_start == cfgnode.bb_start

    # Enable sorting of nodes by start address
    # Assume no overlap (see class docstring)
    def __lt__(self, cfgnode):
        return self.bb_start < cfgnode.bb_start

class RBWriteInjector:
    # Note: trailing comma is significant (tuple vs expression parentheses)
    ASM_BRANCH = ('blx',) # ('bx')
    ASM_PC = () #  ('pop', 'mov', 'adr', 'adrl')
    ASM_LDR = () # ('ldr',)
    ASM_LDM = () # ('ldm',)
    HOTSITE_MARKER = 'HOTSITEIDHERE'
    FORWARDEDGE_MARKER = 'FORWARDEDGELOCATION'
    HOTSITE_FORMAT = '{}'

    def __init__(self, infile, outfile, cfg):
        self.infile = infile
        self.outfile = outfile
        self.cfg = cfg
        self.nodes = sorted([CFGNode(node) for node in cfg.get_nodes()])
        self.edges = cfg.get_edges()
        self.reverse_node_lookup = {cfgnode.node.obj_dict['name']:cfgnode for cfgnode in self.nodes}
        self.funcs = {}
        for i,cfgnode in enumerate(self.nodes, start=1):
            cfgnode.hotsiteid = i
            self.funcs[cfgnode.func_name] = cfgnode.func_addr
        first_inst_addr = int(self.nodes[0].instructions[0].partition(':')[0], 16)
        second_inst_addr = int(self.nodes[0].instructions[1].partition(':')[0], 16)
        # Difference between addresses of two subsequent instructions.
        self.instruction_offset = second_inst_addr - first_inst_addr
        # Address of the current function we're in.
        self.curfunc_name = None
        # Address of the current instruction relative to the currunt function.
        self.curfunc_inst_offset = None
        # Keep track of the offset created by the instructions that have been injected.
        self.injected_offset = 0
        self.main_func = 'main'

    def add_src_and_dst_to_node(self, cfgnode):
        for edge in self.edges:
            src, dst = edge.obj_dict['points'][0], edge.obj_dict['points'][1]
            if src == cfgnode.node.obj_dict['name']:
                cfgnode.pre_nodes.append(self.reverse_node_lookup[dst])
            elif dst == cfgnode.node.obj_dict['name']:
                cfgnode.post_nodes.append(self.reverse_node_lookup[src])

    def node_from_address(self, address):
        # Nodes are sorted by basic block start.
        # Just find the first block with node.end <= address
        for node in self.nodes:
            if address > node.bb_end:
                    continue
            return node

    def patch_up_to_node(self, curnode):
        """ Patch all CFGNodes up to the current one, to account for previously injected code.
        It sets the CFGNode.patched_address to the bb_start address + the injected_offset.

        Call this function just before injecting a new block.
        It will patch all function since the last injection, and update their bb_start addresses.
        This can be done like this, because self.nodes is a sorted list.
        So any node before the PREVIOUS injection will already have been patched,
        with a lower injected_offset value. And wil not be touched again.
        """
        if not curnode:
            return
        for cfgnode in self.nodes:
            if cfgnode.patched_address == 0:
                cfgnode.patched_address = cfgnode.bb_start + self.injected_offset
                if cfgnode == curnode:
                    break


    def get_hotsite_str(self):
        node = self.get_curnode()
        if not node:
            return self.HOTSITE_FORMAT.format(0);
        return self.HOTSITE_FORMAT.format(node.hotsiteid)

    def get_curnode(self):
        if not self.curfunc_name in self.funcs:
            # For functions not in CFG.
            return None
        curfunc_address = self.funcs[self.curfunc_name]
        return self.node_from_address(curfunc_address + self.curfunc_inst_offset)


    def patch_upto_here(self):
        """ Set Node.patched_address for all nodes up-to-and-including this one,
            based on previously injected code.
        """
        self.patch_up_to_node(self.get_curnode())

    def inject_branch(self, blx_target):
        self.patch_upto_here()
        if 'self.icode_branch' not in self.__dict__:
            self.icode_branch = get_icode('Call-C-Function-IndirectBranch.s')
        # Local copy specific to this location
        local_icode = self.icode_branch
        # Insert local forward edge address
        local_icode = local_icode.replace(self.FORWARDEDGE_MARKER, blx_target)
        # Insert local hotsite ID
        local_icode = local_icode.replace(self.HOTSITE_MARKER, self.get_hotsite_str())
        # Add to injected instruction offset
        self.injected_offset += self.instruction_offset * len(local_icode.split('\n'))
        # Write to file.
        self.outfile.write(local_icode)


    def inject_pc(self):
        self.patch_upto_here()
        pass #self.outfile.write('\t;Hotsite: {}                                           ;  <--- Mod PC code here.\n'.format(self.get_hotsite_address()))

    def inject_ldr(self):
        self.patch_upto_here()
        pass #self.outfile.write('\t;Hotsite: {}                                           ;  <--- LDF code here.\n'.format(self.get_hotsite_address()))
    def inject_ldm(self):
        self.patch_upto_here()
        pass #self.outfile.write('\t;Hotsite: {}                                           ;  <--- LDM code here.\n'.format(self.get_hotsite_address()))

    def func_prologue(self, func_name):
        # Set function name and reset instruction offset in function.
        self.curfunc_name = func_name
        # We've already written the PUSH instruction at the start of the function. So set this to one instruction offset.
        self.curfunc_inst_offset = self.instruction_offset
        self.patch_upto_here()
        # Inject code for main or generic function
        if self.curfunc_name == self.main_func:
            if 'icode_setup' not in self.__dict__:
                self.icode_setup = get_icode('Call-C-Function-Setup.s')
            # Add to the injected instruction offset.
            self.injected_offset += self.instruction_offset * len(self.icode_setup.split('\n'))
            # Here we do not check get_curnode() for a None return value, because if the main function is not in
            # the CFG, there is something so fundamentally wrong that the program may just crash and burn.
            self.outfile.write(self.icode_setup.replace('MAINADDRESSHERE', '{}'.format(self.get_curnode().patched_address)))
        #else:
            #if 'icode_func_prologue' not in self.__dict__:
                #self.icode_func_prologue = get_icode('pushpopinjectioncode.s')
            #self.outfile.write(self.icode_func_prologue.replace(self.HOTSITE_MARKER, self.get_hotsite_str()))

    def func_epilogue(self):
        self.patch_upto_here()
        # Inject code for main or generic function
        if self.curfunc_name == self.main_func:
            if 'self.icode_teardown' not in self.__dict__:
                self.icode_teardown = get_icode('Call-C-Function-Epilogue.s')
            self.outfile.write(self.icode_teardown.replace(self.HOTSITE_MARKER, self.get_hotsite_str()))
        else:
            if 'self.icode_func_epilogue' not in self.__dict__:
                self.icode_func_epilogue = get_icode('Call-C-Function-Epilogue.s')
            self.outfile.write(self.icode_func_epilogue.replace(self.HOTSITE_MARKER, self.get_hotsite_str()))
        # Erase function name and instruction offset
        self.curfunc_name = None
        self.curfunc_inst_offset = None

    def parse_file(self):
        """ Parse an assembly file and inject CFI lines.
        Some assumptions are made to simplify the code:
        - The first function label is encountered before any other lines that
          require code to be injected.
        """
        global_name = ''
        lines = [line for line in self.infile]
        line_nr = 0
        while line_nr < len(lines):
            line = lines[line_nr]
            # Skip GCC generated preamble
            splitline = line.split()
            if len(splitline) > 0 and not splitline[0].startswith('.') and not splitline[0].startswith('.') and not splitline[0].startswith('@'):
                # Only act on lines that contain an instruction or function label.
                if  splitline[0][-1] == ':' and line_nr + 1 < len(lines):
                    if not lines[line_nr + 1].strip().startswith('@'):
                        # This is not a function label. Skip it.
                        self.outfile.write(line)
                        line_nr += 1
                        continue
                    # Line is a function label if it:
                    # - does not start with a dot
                    # - ends with a colon
                    # - next line starts with @

                    # Write function label line.
                    self.outfile.write(line)
                    # Increment line counter already
                    line_nr += 1
                    # Fast forward past function preamble (lines with @)
                    while lines[line_nr].strip().startswith('@'):
                        self.outfile.write(lines[line_nr])
                        line_nr += 1
                    # Write the PUSH instruction at the start of the function before injected code.
                    self.outfile.write(lines[line_nr])
                    line_nr += 1
                    self.func_prologue(splitline[0][:-1])
                    # Immediately process next line.
                    continue
                # It is an instruction:
                # Increment function offset
                self.curfunc_inst_offset += self.instruction_offset
                if splitline[0] in RBWriteInjector.ASM_BRANCH:
                    self.inject_branch(splitline[1])
                elif splitline[0] in RBWriteInjector.ASM_PC and line.find('pc') > -1:
                        self.inject_pc()
                elif splitline[0] in RBWriteInjector.ASM_LDR and line.find('pc') > -1:
                        self.inject_ldr()
                elif splitline[0] in RBWriteInjector.ASM_LDM:
                    self.inject_ldm()
                if line_nr + 1 < len(lines):
                    peekline = lines[line_nr + 1]
                    if peekline == '\n' or not peekline[0] == '\t' or peekline[1] == '.':
                        # Next instruction is neither function label, nor instruction.
                        # The current function must have ended.
                        # Inject fuction epilogue and reset variables.
                        self.func_epilogue()
                else:
                    # We've had the last instruction
                    self.func_epilogue()
            # Write the original line to output.
            # sections that inject the original line before their output
            # should end with a continue statement to avoid duplicates.
            line_nr += 1
            self.outfile.write(line)

    def run(self):
        self.parse_file()

    def write_binary_cfg(self):
        for cfgnode in self.nodes:
            self.add_src_and_dst_to_node(cfgnode)
        data = [to_bytes(len(self.nodes))]
        for node in self.nodes:
            data.extend(node.byte_repr())
        try:
            os.mkdir('cfg')
        except FileExistsError:
            pass # Assume it is a directory.
        # Create and open binary CFG file.
        with open('{}.bin'.format(os.path.join('cfg',os.path.splitext(os.path.basename(self.infile.name))[0])), 'wb') as bincfg:
            for value in data:
                bincfg.write(value)

def main():
    parser = argparse.ArgumentParser(description='Inject code to allow control flow integrity (CFI) checking into a program\'s assembly, based on the program\'s control flow graph(CFG)')
    parser.add_argument('input' , help='The file to read as input', type=argparse.FileType('r'))
    parser.add_argument('dot'   , help='The CFG for the program, a dot file.')
    parser.add_argument('output', help='The file to write to as output, the default is stdout.', nargs='?', type=argparse.FileType('w'), default=sys.stdout)
    args = parser.parse_args()
    sys.stderr.write('The input file is:  {}\nThe dot file is: {}\nThe output file is:   {}\n'.format(args.input.name, args.dot, args.output.name))
    cfg = pydot.graph_from_dot_file(args.dot)
    # newer pydot version always returns list.
    if isinstance(cfg, list):
        cfg = cfg[0]
    injector = RBWriteInjector(args.input, args.output, cfg)
    injector.run()
    injector.write_binary_cfg()
    args.output.close()
    args.input.close()
if __name__ == '__main__':
    main()
