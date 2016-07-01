"""
Created on Tue Jun 22 14:37:04 2016

@author: Gijsbert ter Horst

"""
import gcc
import os
import sys
from gccutils import get_src_for_loc, cfg_to_dot, callgraph_to_dot
from gccutils.graph.supergraph import Supergraph
from subprocess import Popen, PIPE

CDIR = 'cfg'
FORMAT = 'svg'

class ShowGimple(gcc.GimplePass):
    def execute(self, fun):
        # (the CFG should be set up by this point, and the GIMPLE is not yet
        # in SSA form)
        if fun and fun.cfg:
            dot = cfg_to_dot(fun.cfg, fun.decl.name)
            filename = os.path.join(CDIR, fun.decl.name)
            with open(filename + '.dot','w') as dotfile:
                dotfile.write(dot)
                print('Wrote dot file: {}.dot'.format(filename), file=sys.stderr)
            Popen(['dot', '-T' + FORMAT, '-o', '{}.{}'.format(filename, FORMAT)], stdin=PIPE).communicate(dot.encode('utf-8'))
            print('Wrote: {}.{}\n'.format(filename,FORMAT), file=sys.stderr)
#            for block in fun.cfg.basic_blocks:
#                print('Block {}:'.format(block.index))
#                for gimple in block.gimple:
#                    if isinstance(gimple, gcc.GimpleCall):
#                        pass

class ShowSupergraph(gcc.IpaPass):
    def execute(self):
        # (the callgraph should be set up by this point)
        if gcc.is_lto():
            sg = Supergraph(split_phi_nodes=False,
                            add_fake_entry_node=False)
            filename = os.path.join(CDIR, 'supergraph')
            dot = sg.to_dot('supergraph')
            with open(filename + '.dot','w') as dotfile:
                dotfile.write(dot)
                print('Wrote dot file: {}.dot'.format(filename), file=sys.stderr)
            Popen(['dot', '-T' + FORMAT, '-o', '{}.{}'.format(filename, FORMAT)], stdin=PIPE).communicate(dot.encode('utf-8'))
            print('Wrote: {}.{}\n'.format(filename,FORMAT), file=sys.stderr)

ps = ShowGimple(name='show-gimple')
ps.register_after('cfg')
ps = ShowSupergraph(name='show-supergraph')
ps.register_before('whole-program')