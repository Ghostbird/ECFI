"""
Created on Tue Jun 22 14:37:04 2016

@author: Gijsbert ter Horst

"""
import gcc
import os
from gccutils import get_src_for_loc, cfg_to_dot
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
                print('Wrote dot file: {}.dot'.format(filename))
            Popen(['dot', '-T' + FORMAT, '-o', '{}.{}'.format(filename, FORMAT)], stdin=PIPE).communicate(dot.encode('utf-8'))
            print('Wrote: {}.{}\n'.format(filename,FORMAT))

ps = ShowGimple(name='show-gimple')
ps.register_after('cfg')

