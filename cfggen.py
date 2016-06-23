"""
Created on Tue Jun 22 14:37:04 2016

@author: Gijsbert ter Horst

"""
import gcc
from gccutils import get_src_for_loc, cfg_to_dot
from subprocess import Popen, PIPE

class ShowGimple(gcc.GimplePass):
    def execute(self, fun):
        # (the CFG should be set up by this point, and the GIMPLE is not yet
        # in SSA form)
        if fun and fun.cfg:
            dot = cfg_to_dot(fun.cfg, fun.decl.name)
            Popen(['dot', '-Tsvg', '-o', fun.decl.name], stdin=PIPE)

ps = ShowGimple(name='show-gimple')
ps.register_after('cfg')

