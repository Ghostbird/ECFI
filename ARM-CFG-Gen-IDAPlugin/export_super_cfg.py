#Use the following two lines in case you do not have networkx in your IDA Pro. I used anaconda2 for IDA and added the path to the path virtual env.
#import sys
#sys.path.append(C:\Users\syssec\Anaconda2\lib\site-packages)
from idautils import *
from idc import *
import idaapi
#from idaapi import *
#idaapi.require("networkx")
import networkx as nx
import re
#pydotplus
class NxBB:
    def __init__(self, start, end, retn=False, entry=False, fname=None):
        self.start = start
        self.end = end
        self.retn_bb = retn
        self.entry_bb = entry
        self.fname = fname
    def __str__(self):
        return "(" + self.fname + ", " + hex(self.end) + ")"


def extract_bbs_from_function(G, funcEA):
    f = idaapi.get_func(funcEA)
    fc = idaapi.FlowChart(f)
    fname = GetFunctionName(funcEA)
    for bb in fc:
        start, end = bb.startEA, bb.endEA
        retn_bb = False
        entry_bb = False
        for inst in Heads(bb.startEA, bb.endEA):
 #           if GetMnem(inst) == "call":		
            if GetMnem(inst) == "BL" or GetMnem == "BLX" or GetMnem == "BX" or GetMnem == "B" or re.search("LDR {R0, .*}",GetDisasm(inst)) != None:
                print("This is call", GetMnem(inst), hex(inst), fname)
				# this is the real end of the bb
                if start == funcEA:
                    # this is the first block of a function
                    entry_bb = True
                G.add_node(start, data=NxBB(start, ItemEnd(inst), entry=entry_bb, fname=fname))
                G.add_edge(start, ItemEnd(inst), call=False)
                start = ItemEnd(inst)
            if  re.search("POP {PC, .*}",GetDisasm(inst))!=None or re.search("LDR {PC, .*}",GetDisasm(inst))!=None or GetMnem(inst)=={"LDMFD   SP!, {R11,PC}"} :
                retn_bb = True
        entry_bb = False
        if start == funcEA:
            # this is the first block of a function
            entry_bb = True
        G.add_node(start, data=NxBB(start, end, retn=retn_bb, entry=entry_bb, fname=fname))
        for e in bb.succs():
            G.add_edge(start, e.startEA, call=False)

def export_xrefs_edges(G, ea):
    lable_counter = 0
    for func_ea in Functions(SegStart(ea), SegEnd(ea)):
        fname = GetFunctionName(func_ea)
        
        # get all retn_bbs from this funciton
        retn_bbs = []
        for node in G.nodes():
            if G.node[node]['data'].fname == fname and G.node[node]['data'].retn_bb:
                retn_bbs.append(node)
                
        for ref_ea in CodeRefsTo(func_ea, 0):
            #caller_name = GetFunctionName(ref_ea)
            if GetMnem(ref_ea) != "BL" and GetMnem(ref_ea) != "BLX" and GetMnem(ref_ea) != "B" and GetMnem(ref_ea) != "BX":
                print GetMnem(ref_ea) 
                # 
                print "IGNORED: ", hex(ref_ea), GetDisasm(ref_ea)
                # don't add back edge if the ref is not comming from a I
                continue
            bb_end = ItemEnd(ref_ea)
            
            # add return edges
            for retn_bb in retn_bbs:
                G.add_edge(retn_bb, bb_end, lable=lable_counter, ret=True)

            
            # search the start of the bb
            # TODO: make this faster (maybe with successor function?)
            for node in G.nodes():
                if G.node[node]['data'].end == bb_end:
                    G.add_edge(G.node[node]['data'].start, func_ea, call=True, lable=lable_counter)
                    #G.add_edge(G.node[node]['data'].start, func_ea, call=True)
                    break
            lable_counter += 1


#just give one function address
tfunc = ScreenEA()
#graph for networkx
G = nx.DiGraph()
for func in Functions(SegStart(tfunc), SegEnd(tfunc)):
    extract_bbs_from_function(G, func)
export_xrefs_edges(G, tfunc)


nx.nx_pydot.write_dot(G, "test2.dot")
print G.nodes()
