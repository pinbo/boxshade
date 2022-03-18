#!/usr/bin/env python

from itertools import chain
import numpy as np
# import BS_config as BS
# from platform import system
# from collections import namedtuple
# import datetime

# Col = namedtuple("red", "green", "blue")

settings = {
    "simsline": 'SIMS:F YW:Y FW:W FY:I LM:L IM:M IL:R KH:K RH:H KR:A G:S T:D EN:E DQ:N EQ:P G:V M:END',
    "grpsline": 'GRPS:FYW:ILVM:DE:GA:ST:NQ:RKH:END',
    "DNAsimsline": 'SIMS:A GR:G AR:C TY:T CY:R AG:Y CT:END',
    "DNAgrpsline": 'GRPS:AGR:CTY:END',
    "thrfrac": 0.7,
    "scflag": False,
    "countGaps": True,
    "snameflag": True,
    "RHsnumsflag": False,
    "LHsnumsflag": True,
    "defnumsflag": True,
    "simflag": True,
    "globalflag": True,
    "consflag": False,
    "outlen": 60,
    "interlines": 1,
    "symbcons": ' LU',
    "consline": 1,
    "rulerflag": True,
    "pepseqsflag": True,
    "PSfgds": [(0, 0, 0), (255, 255, 255), (255, 255, 255), (255, 255, 255)],
    "PSbgds": [(255, 255, 255), (0, 0, 0), (180, 180, 180), (0, 0, 0)],
    "PSFsize": 10,
    "PSLCs": [False, False, False, False],
    "PSlandscapeflag": False,
    "ASCIIchars": ['L', '.', 'l', '*'],
    "pos": (200, 200),
    "size": (400, 400),

}


# class object that will handle output to a file
# will subclass this for output to RTF/ASCII/PDF
# noinspection PyMethodMayBeStatic
class Filedev():
    def __init__(self):
# create the reference points for instance variables that will hold all the data to be processed by this instance
        self.seqs = np.array([[' ', ' '], [' ', ' ']], dtype=str)
        self.cols = np.full(self.seqs.shape, 0, dtype=np.int32)
        self.seqnames =[]
        self.no_seqs = 0
        self.LHprenums = []
        self.RHprenums = []
        self.seqlens = []
        self.startnums = []
        self.lcs = []
        self.interlines = 0
        self.file_filter = ("All files (*);;PDF files (*.pdf);;RTF files (*.rtf);;ASCII files (*.txt)")
        self.file = None
        self.outstream = ""
        self.parent = None


    def rgb(self, col):
        # return col.red(), col.green(), col.blue()
        return col[0], col[1], col[2]


    def make_lowercase(self, rulerflag):
        if self.lcs[0]:
            self.seqs[self.cols==0]=np.char.lower(self.seqs[self.cols==0])
        if self.lcs[1]:
            self.seqs[self.cols==1]=np.char.lower(self.seqs[self.cols==1])
        if self.lcs[2]:
            self.seqs[self.cols==2]=np.char.lower(self.seqs[self.cols==2])
        if self.lcs[3]:
            self.seqs[self.cols==3]=np.char.lower(self.seqs[self.cols==3])

class RTFdev(Filedev):
    def __init__(self, fname):
        super(RTFdev, self).__init__()
        self.file_filter = ("RTF files (*.rtf);;All files (*)")
        self.Alignment = fname
        self.bgds = settings["PSbgds"]
        self.fgds = settings["PSfgds"]
        self.FSize = settings["PSFsize"]
        self.lcs = settings["PSLCs"]
        simflag = settings["simflag"]
        globalflag = settings["globalflag"]
        if not simflag:
            self.fgds[2] = self.fgds[0]
            self.bgds[2] = self.bgds[0]
            self.lcs[2] = self.lcs[0]
        if not globalflag:
            self.fgds[3] = self.fgds[1]
            self.bgds[3] = self.bgds[1]
            self.lcs[3] = self.lcs[1]

        dev_miny = 1.0
        dev_maxy = 15000.0
        dev_ysize = self.FSize * 20.0
        self.lines_per_page = int((dev_maxy - dev_miny) / dev_ysize)

    def graphics_init(self):
        self.outstream += '{\\rtf1\\ansi\\deff0\n{\\fonttbl{\\f0\\fmodern Courier New;}}\n'
        self.outstream += '{{\\info{{\\author BOXSHADE}}{{\\title {}}}}}\n'.format(self.Alignment)
        self.outstream += '{\\colortbl\n'
        for i in range(4):
            self.outstream += '\\red{}\\green{}\\blue{};'.format(*self.rgb(self.fgds[i]))

        self.outstream += '\\red0\\green0\\blue0;' # equivalent to fgds[4]
        for i in range(4):
            self.outstream += '\\red{}\\green{}\\blue{};'.format(*self.rgb(self.bgds[i]))

        self.outstream += '\\red255\\green255\\blue255;}\n' # equivalent to bgds[4]
        self.outstream += '\\paperw11880\\paperh16820\\margl1000\\margr500\n'
        self.outstream += '\\margt910\\margb910\\sectd\\cols1\\pard\\plain\n'
        self.outstream += '\\fs{}\n\\b\n'.format(self.FSize * 2)
        # print("fisrt outstream print\n")
        # print(self.outstream)
        return True

    def set_colour(self, c):
        colno = c
        self.outstream += '\n\\chshdng0\\chcbpat{0}\\cb{0}\\cf{1} '.format(5+colno, colno)

    def char_out(self,c):
        self.outstream += c

    def string_out(self, str):
        self.outstream += str

    def newline(self):
        self.outstream += '\n\\cb{}\\cf{} \\line'.format(9, 4)

    def newpage(self):
        self.outstream += '\\page'

    def exit(self):
        self.outstream += '\\b0}\n'
        outfile = open(self.Alignment,'w')
        outfile.write(self.outstream)
        outfile.close()
        # print(self.outstream)

## read fasta file
## function to extract sequences from a fasta file 
def get_fasta(infile):
	fasta = {} # dictionary for alignment
	with open(infile) as file_one:
		for line in file_one:
			line = line.strip()
			if line: # skip blank lines
				if line.startswith(">"):
					sequence_name = line.lstrip("> ").split()[0] # left strip > or space, so " > abc edf" will be "abc edf", then split by space to get "abc"
					fasta[sequence_name] = ""
				else:
					fasta[sequence_name] += line.replace(" ", "") # remove spaces in case
	return fasta

##
aaset = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
lenaa = len(aaset)
aasetlow = 'abcdefghijklmnopqrstuvwxyz'
chars = aaset+aasetlow
gapchars = '-.~'

aa_dict = dict(zip(list(aaset), range(1,lenaa+1)))
aalow_dict = dict(zip(list(aasetlow), range(1,len(aasetlow)+1)))

simtable = np.full((lenaa,lenaa), False, dtype=bool)
for i in range(lenaa):
    simtable[i,i]= True
grptable = np.copy(simtable)

def sim(a, b):
    p1 = aa_dict[a] if a in aa_dict else False
    p2 = aa_dict[b] if b in aa_dict else False
    if p1 and p2:
        return simtable[p1, p2]
    else:
        return False

def grp(a, b):
    p1 = aa_dict[a] if a in aa_dict else False
    p2 = aa_dict[b] if b in aa_dict else False
    if p1 and p2:
        return grptable[p1, p2]
    else:
        return False

def readsims():
    global simtable
    global grptable
    if settings["pepseqsflag"]:
        simsline = settings["simsline"]
        grpsline = settings["grpsline"]
    else:
        simsline = settings["DNAsimsline"]
        grpsline = settings["DNAgrpsline"]
    simsline = simsline.split(":")
    grpsline = grpsline.split(":")
    simtable = np.full((lenaa, lenaa), False, dtype=bool)
    for i in range(lenaa):
        simtable[i, i] = True
    grptable = np.copy(simtable)
    for i in range(1,len(simsline)-1):
        p1=aa_dict[simsline[i][0]] if simsline[i][0] in aa_dict else False
        if p1:
            for j in range (2,len(simsline[i])):
                p2 = aa_dict[simsline[i][j]] if simsline[i][j] in aa_dict else False
                if p2 :
                    simtable[p1,p2] = True
                    simtable[p2,p1] = True

    for k in range(1,len(grpsline)-1):
        for j in range(0, len(grpsline[k])-1):
            p1 = aa_dict[grpsline[k][j]] if grpsline[k][j] in aa_dict else False
            if p1 :
                for i in range (j+1, len(grpsline[k])):
                    p2 = aa_dict[grpsline[k][i]] if grpsline[k][i] in aa_dict else False
                    if p2 :
                        grptable[p1, p2] = True
                        grptable[p2, p1] = True
    return
##
class align:
    def __init__(self, fasta):
        self.al = fasta
        self.seqs = np.array([list(fasta[rec].upper()) for rec in fasta], str, order="F")
        self.seqnames = [rec for rec in fasta]
        self.no_seqs = self.seqs.shape[0]
        self.maxseqlen = self.seqs.shape[1]
        self.cols = np.full(self.seqs.shape, 0, dtype=np.int32)
        self.cons = np.full(self.maxseqlen, ' ', dtype=str)
        self.conschar = np.copy(self.cons)
        self.seqlens = np.full(self.no_seqs, self.maxseqlen, dtype=np.int32)
        self.startnums = np.full(self.no_seqs, 1, dtype=np.int64)
        self.consenslen = 0
        for i in range(0,self.no_seqs):
            while self.seqlens[i] and (self.seqs[i, self.seqlens[i] - 1] in [' ', '-', '.']):
                self.seqlens[i] -= 1
            if self.seqlens[i] > self.consenslen:
                self.consenslen = self.seqlens[i]
        
        #
    def make_consensus(self):
        thrfrac = settings["thrfrac"]
        scflag = settings["scflag"]
        countGaps = settings["countGaps"]
        self.pepseqsflag = settings["pepseqsflag"]
        readsims() # (re)-read the sims/grps and remake the simtable and grptable, in case settings have changed
        # procedure to make a consensus which forms the basis of the shading
        thr = round(thrfrac*self.no_seqs)
        if not scflag:
            uf=np.frompyfunc(grp,2,1)
            for i in range(0, self.consenslen):
                x = self.seqs[:, i]
                xx=np.char.isalpha(x)
                if not countGaps:
                    thr = round(thrfrac*np.sum(xx))
                idcount = np.sum(x == x[xx,None], 0)
                grpcount = np.sum(uf(x, x[xx,None]).astype(bool), 0)
                self.cons[i] = ' '
                maxidcount = np.amax(idcount)
                maxgrpcount = np.amax(grpcount)
                idindex = list(chain.from_iterable(np.where(idcount == maxidcount)))
                grpindex = list(chain.from_iterable(np.where(grpcount == maxgrpcount)))
                # if there is a single maxid, and it is greater than the threshold, that is the consensus
                if maxidcount >= thr :
                    if len(idindex) == 1 :
                       self.cons[i] = x[idindex[0]]
                    # there is not a single maxidcount, but they may all be the same residue
                    # Have to check to see if all the residues with maxidcount are the same
                    else:
                        if np.all(x[idindex[0]]==x[idindex]):
                            self.cons[i] = x[idindex[0]]
                #if there is an equally high idcount for a different residue then there can't be a
                # single residue consensus, so look for a group consensus
                elif maxgrpcount >= thr:
                    if len(grpindex) == 1:
                        self.cons[i] = x[grpindex[0]]
                    elif np.all(uf(x[grpindex[0]], x[grpindex]).astype(bool)):
                        (vv,cc) = np.unique(x[grpindex], return_counts=True)
                        self.cons[i] = vv[np.argmax(cc)].lower()
        # if maxsimcount is not unique and the other residue is NOT in the same
        # similarity group then there is so consensus based on similarity. If
        # the two residues with the same similarity score are in the same
        # similarity group, flag that consensus position by making the
        # residue lowercase

        else:
        # this 'else' means that the scflag (make specific sequence the consensus) is true, so copy the sequence at row self.consensnum-1 into cons[]
            np.copyto(self.cons[0:self.consenslen], self.seqs[self.consensnum-1, 0:self.consenslen])

        return # from make_consensus

    def make_colours(self):
    # The array of "colours" defines the shading that will be applied to each array
        # settings = QSettings("Boxshade", "Boxshade")
        thrfrac = settings["thrfrac"]
        scflag = settings["scflag"]
        consflag = settings["consflag"]
        symbcons = settings["symbcons"]
        countGaps = settings["countGaps"]
        thr = round(thrfrac * self.no_seqs)
        self.cols.fill(0)
        seqcount = self.no_seqs
        for i in range(0, self.consenslen):
            idcount = 0
            simcount = 0
            aasetflag = self.cons[i] in aaset
            if not countGaps:
                seqcount=np.sum(np.char.isalpha(self.seqs[:, i]))
                thr = round(thrfrac * seqcount)

            # count similar residues for the case of a group consensus;
            # note that in this case there cannot be any residues marked as
            # 'identical', by definition
            if self.cons[i] in aasetlow : # the np test here is 100x slower
                for j in range(0, self.no_seqs) :
                    if grp(self.seqs[j,i], self.cons[i].upper()):
                        self.cols[j,i] = 2
                        simcount += 1
            else:
                for j in range(0, self.no_seqs):
                    if self.seqs[j,i] == self.cons[i]:
                        idcount += 1
                    elif sim(self.seqs[j,i], self.cons[i]):
                        simcount += 1
                if (idcount == seqcount) and aasetflag:
                    if countGaps:
                        self.cols[:,i] = 3 # if all sequences same colour them identical idcount+
                    else:
                        for j in range(0, self.no_seqs):
                            if self.seqs[j, i] in gapchars:
                                self.cols[j, i] = 0  # a gap, always uncoloured
                            else:
                                self.cols[j, i] = 3
                elif ((idcount+simcount) >= thr) and aasetflag:
                    for j in range(0, self.no_seqs):
                        if self.seqs[j,i] == self.cons[i]:
                            self.cols[j,i] = 1 # a consensus residue
                        elif sim(self.seqs[j,i], self.cons[i]):
                            self.cols[j,i] = 2 # similar to the consensus

            if consflag: # should generate a consensus line
                        # in Python, there is no way to do this except with multiple nested if/elif/elses
                if idcount == seqcount:
                    symbchar=symbcons[2].upper()
                    if symbchar == 'U':
                        self.conschar[i] = self.cons[i].upper()
                    elif symbchar == "L":
                        self.conschar[i] = self.cons[i].lower()
                    elif symbchar == "B" or symbchar == " ":
                        self.conschar[i] = " "
                    else: self.conschar[i] =symbcons[2]
                elif (idcount+simcount)>=thr :
                    symbchar = symbcons[1].upper()
                    if symbchar == 'U':
                        self.conschar[i] = self.cons[i].upper()
                    elif symbchar == "L":
                        self.conschar[i] = self.cons[i].lower()
                    elif symbchar == "B" or symbchar == " ":
                        self.conschar[i] = " "
                    else: self.conschar[i] = symbcons[1]
                else:
                    symbchar = symbcons[0].upper()
                    if symbchar == 'U':
                        self.conschar[i] = self.cons[i].upper()
                    elif symbchar == "L":
                        self.conschar[i] = self.cons[i].lower()
                    elif symbchar == "B" or symbchar == " ":
                        self.conschar[i] = " "
                    else: self.conschar[i] = symbcons[0]
        return # end of the function make_colours

    def process_seqs(self):
        if self.no_seqs < 2:
            return
        self.make_consensus()
        self.make_colours()

    def prep_out(self, gr_out):
        self.LHsnumsflag = settings["LHsnumsflag"]
        self.RHsnumsflag = settings["RHsnumsflag"]
        self.defnumsflag = settings["defnumsflag"]

        self.scflag = settings["scflag"]
        self.consflag = settings["consflag"]
        self.snameflag = settings["snameflag"]
        self.globalflag = settings["globalflag"]
        self.outlen = settings["outlen"]
        self.interlines = settings["interlines"]
        self.rulerflag = settings["rulerflag"]

        sname_just = max((self.consflag*9), max(map(len, self.seqnames)))
        nseqs = self.no_seqs
        if self.consflag:
            nseqs += 1
        if self.rulerflag:
            nseqs += 1
            gr_out.seqnames.append(" ".ljust(sname_just))
        gr_out.seqs = np.full((nseqs, self.seqs.shape[1]), ' ', dtype=str)
        gr_out.cols = np.full((nseqs, self.seqs.shape[1]), 0, dtype=np.int32)
        gr_out.seqlens = np.full(nseqs, 0, dtype=np.int64)
        gr_out.seqnames.extend([name.ljust(sname_just) for name in self.seqnames])
        if self.consflag:
            gr_out.seqnames.append("consensus".ljust(sname_just))
        if self.rulerflag: #code here to create ruler and put it in first line of gr_out.seqs
            gr_out.seqlens[0] = self.consenslen
            np.copyto(gr_out.seqs[0], np.array(['.']))
            gr_out.seqs[0,list(range(4,self.consenslen,10))] = ':'
            np.copyto(gr_out.cols[0], np.array([4]))
            gr_out.seqs[1,0] = '1'
            for i in range(10, self.consenslen+1, 10):
                inum = list(str(i))
                li = len(inum)
                for j in range(li):
                    gr_out.seqs[0,(j+i-li)]=inum[j]
        np.copyto(gr_out.seqs[self.rulerflag:self.rulerflag+self.no_seqs, :], self.seqs)
        np.copyto(gr_out.cols[self.rulerflag:self.rulerflag+self.no_seqs, :], self.cols)
        np.copyto(gr_out.seqlens[self.rulerflag:self.rulerflag+self.no_seqs], self.seqlens)
        if self.scflag and (self.consensnum >0):
            np.copyto(gr_out.cols[self.consensnum-1+self.rulerflag], np.array([4]))
        if self.consflag:
            np.copyto(gr_out.seqs[nseqs-1,:], self.conschar)
            np.copyto(gr_out.cols[nseqs-1,:], np.array([4]))
            gr_out.seqlens[nseqs-1] = self.consenslen

        gr_out.no_seqs = self.no_seqs
        gr_out.make_lowercase(self.rulerflag)
        gr_out.no_seqs = nseqs
        if self.LHsnumsflag or self.RHsnumsflag:
            gr_out.startnums = np.full(nseqs, 0, dtype=np.int64)
            if self.rulerflag:
                gr_out.startnums[0] = 1
            np.copyto(gr_out.startnums[self.rulerflag:self.rulerflag + self.no_seqs], self.startnums)
            if self.consflag:
                gr_out.startnums[nseqs - 1] = 1
            nblocks = (self.consenslen//self.outlen)+1
            gr_out.LHprenums = [['' for i in range(nblocks)] for j in range(nseqs)]
            gr_out.RHprenums = [['' for i in range(nblocks)] for j in range(nseqs)]
            numlen = len(str(np.amax(self.startnums)+self.consenslen))
            if self.rulerflag:
                gr_out.LHprenums[0] = [' ' * numlen for x in gr_out.LHprenums[0]]
                gr_out.RHprenums[0] = [' ' * numlen for x in gr_out.RHprenums[0]]
            for i in range(self.rulerflag, nseqs-self.consflag):
                totcount=gr_out.startnums[i]
                bn = 0
                thisline = 0
                for j in range(0, self.consenslen):
                    if gr_out.seqs[i,j] in chars:
                        thisline +=1
                    if ((j+1) % self.outlen == 0) or ((j==self.consenslen-1) and (bn == nblocks-1)): # end of a line or last block
                        if (totcount == gr_out.startnums[i]) and (thisline == 0): # no chars yet
                            gr_out.LHprenums[i][bn] = ' ' * numlen
                            gr_out.RHprenums[i][bn] = ' ' * numlen
                        elif (j > gr_out.seqlens[i]) and (thisline == 0): # empty stuff at the end
                            gr_out.LHprenums[i][bn] = ' ' * numlen
                            gr_out.RHprenums[i][bn] = ' ' * numlen
                        else:
                            gr_out.LHprenums[i][bn] = str(totcount).rjust(numlen)
                            totcount += thisline
                            gr_out.RHprenums[i][bn] = str(totcount-1).rjust(numlen)
                        bn += 1
                        thisline = 0
            if self.consflag:
                consl = nseqs - 1
                gr_out.LHprenums[consl] = [' ' * numlen for x in gr_out.LHprenums[consl]]
                gr_out.RHprenums[consl] = [' ' * numlen for x in gr_out.RHprenums[consl]]
        if not gr_out.graphics_init():
            # QApplication.restoreOverrideCursor()
            return False
        else:
            return True
        # at the end out prep_out, the formatted set of seqs/ruler, consensus, etc are stored in the output object

    def do_out(self,gr_out):
        nblocks = ((self.consenslen -1)// self.outlen)+1
        ll = (nblocks*(gr_out.no_seqs+self.interlines))-self.interlines
        lcount = 0
        for i in range(0, nblocks):
            for j in range(0, gr_out.no_seqs):
                if self.snameflag:
                    gr_out.set_colour(4)
                    gr_out.string_out(gr_out.seqnames[j]+' ')
                if self.LHsnumsflag:
                    gr_out.set_colour(4)
                    gr_out.string_out(gr_out.LHprenums[j][i]+' ')
                io=i*self.outlen
                for k in range(self.outlen):
                    m = io+k
                    if m <= self.consenslen-1:
                        gr_out.set_colour(gr_out.cols[j,m])
                        gr_out.char_out(gr_out.seqs[j,m])
                if self.RHsnumsflag:
                    gr_out.set_colour(4)
                    gr_out.string_out(' '+gr_out.RHprenums[j][i])
                lcount +=1
                if lcount >= gr_out.lines_per_page:
                    ll -= lcount
                    lcount = 0
                    gr_out.newpage()
                else:
                    gr_out.newline()
            if (lcount + gr_out.no_seqs + self.interlines) <= gr_out.lines_per_page:
                for j in range(self.interlines):
                    gr_out.newline()
                    lcount +=1
            else:
                ll -= lcount
                lcount = 0
                gr_out.newpage()
        gr_out.exit()
        # QApplication.restoreOverrideCursor()

    def RTF_out(self, outfileName):
        if self.no_seqs < 2:
            return
        if not outfileName.endswith(".rtf"):
            outfileName += ".rtf"
        gr_out = RTFdev(outfileName)
        if not self.prep_out(gr_out):
            return
        self.do_out(gr_out)

if __name__ == '__main__':
    import sys
    if len(sys.argv) < 3:
        print("\nThis program takes an aligned fasta file and write a RTF file.\nIt needs 2 input: 1) the name of the aligned fasta file and 2) the output file name.\n")
        exit()
    fasta = get_fasta(sys.argv[1])
    newBS = align(fasta)
    newBS.process_seqs()
    newBS.RTF_out(sys.argv[2])

