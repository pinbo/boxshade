# boxshade
This is a modified version of the original C version boxshade.

The original boxshade website is not accessible anymore, so I found the C source at [OpenSUSE's repository](https://ftp.lysator.liu.se/pub/opensuse/repositories/science/openSUSE_Leap_15.3/src/boxshade-3.3.1-lp153.1.9.src.rpm).

I made several modifications to make it work better, including:

- Fix the adding ruler bug (infinite loop)
- support for fasta file input
- add more command line options
- add a makefile for linux (modified version of makefile.unx)

A webassembly version can be found here:
https://junli.netlify.app/apps/boxshade/

# Compile

## compile for local use
I only tested it in Linux system.
```sh
# to compile
make
# to clean
make clean
```
The you can copy the binary `boxshade` and all the files in the settings directory to the working directory, because boxshade will look for the .par, .sim and .grp file in the working directory by default unless you tell it where they are.

`boxshade` needs an aligned peptide (default) or DNA (-dna flag) as input. To avoid being asked many questions, you can use the "*-def*" flag. Example command for peptide input and eps output:

`./boxshade -in=test.aln -out=test.eps -dev=2 -par=./settings/box_pep.par -sim=./settings/box_pep.sim -grp=./settings/box_pep.grp -def`

For RTF output:

`./boxshade -in=test.aln -out=test.rtf -dev=4 -par=./settings/box_pep.par -sim=./settings/box_pep.sim -grp=./settings/box_pep.grp -def`

For DNA and RTF output:

`./boxshade -in=test.aln -out=test.rtf -dev=4 -par=./settings/box_dna.par -sim=./settings/box_dna.sim -grp=./settings/box_dna.grp -def`

If the setting files are in the working directory, you can omit "*-par=./settings/box_dna.par -sim=./settings/box_dna.sim -grp=./settings/box_dna.grp*".

## compile for webassembly
You need to install [Emscripten](https://emscripten.org/docs/getting_started/downloads.html) first. Then use the command below to compile it:

`emmake make CC="emcc" EXE=".js" LIBS="-lm -s FORCE_FILESYSTEM=1 -s EXPORTED_RUNTIME_METHODS=[\"callMain\"] -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 --preload-file settings@/"`

I have made an webapp here: https://junli.netlify.app/apps/boxshade/

# Acknowledgement

I thank the authors of boxshade. Below is from `box.c`:
```
This program might still contain bugs and is not particularly
user-friendly. It is completely public-domain and may be
passed around and modified without any notice to the author.

the authors addresses are:

  Kay Hofmann                              Michael D. Baron
  Bioinformatics Group                     BBSRC Institute for Animal Health
  ISREC                                    Pirbright, Surrey GU24 0NF
  CH-1066 Epalinges s/Lausanne             U.K.
  Switzerland

C port by

  Hartmut Schirmer
  Technische Fakultaet
  Kaiserstr. 2
  D-24143 Kiel
  Germany

for any comments write an E-mail to
  Kay:     khofmann@isrec-sun1.unil.ch
  Michael: baron@bbsrc.ac.uk (though kay wrote 99% of the program and is more
	      likely to be of help, especially with input and output problems)
  Hartmut: hsc@techfak.uni-kiel.de (don't send Kay or Michael any questions
				    concerning the 'C' version of boxshade)
```