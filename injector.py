"""
Created on Tue Jun 14 16:55:52 2016

@author: Gijsbert ter Horst

See: https://community.arm.com/groups/processors/blog/2013/09/25/branch-and-call-sequences-explained
for information on branching in ARM.
"""
import sys
import argparse
import fileinput

class RBWriteInjector:
    ASM_MAIN = ('main:')
    ASM_BRANCH = ('b', 'bl', 'bx', 'blx')
    ASM_PC = ('pop', 'mov', 'adr', 'adrl')
    ASM_LDR = ('ldr')
    ASM_LDM = ('ldm')

    def __init__(self, infilename, outfilename):
        self.infilename = infilename
        self.outfilename = outfilename
        self.infile = None
        self.outfile = None

    def inject_setup(self):
        self.outfile.write('\tSetup code here.\n')

    def inject_branch(self):
        self.outfile.write('\tBranch code here.\n')

    def inject_pc(self):
        self.outfile.write('\tMod PC code here.\n')

    def inject_ldr(self):
        self.outfile.write('\tLDR code here.\n')

    def inject_ldm(self):
        self.outfile.write('\tLDM code here.\n')

    def parse_file(self):
        for line in self.infile:
            if line != '\n':
                splitline = line.split()
                if splitline[0] in RBWriteInjector.ASM_BRANCH:
                    self.inject_branch()
                    self.outfile.write(line)
                elif splitline[0] in RBWriteInjector.ASM_PC:
                    if splitline[1].startswith('pc'):
                        self.inject_pc()
                    self.outfile.write(line)
                elif splitline[0] in RBWriteInjector.ASM_LDR:
                    if splitline[1].startswith('pc'):
                        self.inject_ldr()
                    self.outfile.write(line)
                elif splitline[0] in RBWriteInjector.ASM_LDM:
                    self.inject_ldr()
                    self.outfile.write(line)
                elif splitline[0] in RBWriteInjector.ASM_MAIN:
                    self.outfile.write(line)
                    self.inject_setup()
                else:
                    self.outfile.write(line)

    def run(self):
        with fileinput.input(files=self.infilename) as self.infile:
            with sys.stdout if self.outfilename == '-' else open(self.outfilename, 'w') as self.outfile:
                self.parse_file()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('input',help='The file to read as input, the default is stdin. Passing the character - (dash) will also read from stdin.', default='-')
    parser.add_argument('output',help='The file to write to as output, the default is stdout. Passing the character - (dash) will also read from stdout.', default='-')
    args = parser.parse_args()
    sys.stderr.write('The input file is: {}\nThe output file is: {}\n'.format(args.input, args.output))
    injector = RBWriteInjector(args.input, args.output)
    injector.run()

if __name__ == '__main__':
    main()
