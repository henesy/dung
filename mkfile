</$objtype/mkfile

LIBS = 

TARG = dungeon_generator

BIN = $home/bin/$objtype

OFILES = main.$O \
		binheap.$O \
		monsters.$O \
		world.$O \
		printing.$O

HFILES = binheap.h \
		dungeon_generator.h

</sys/src/cmd/mkone
