# extract executables for KULT
#
#	file structure
#	+0	W		(BE) # of resources (N)
#	+2	D[N]	(BE) array of resource offsets (from the end of this array)
#
#	resource structure:
#	+0	D		(BE) size of packed resource data (P)
#	+4	D		(BE) size of unpacked resource data
#	+8	B[P]	resource data (divided in contiguous blocks, see below)
#
#	block structure
#	+0	B		dictionary size [R] (0 for literal blocks, > 0 for packed blocks)
#	+1	B		continue flag (0 for the last block)
#	+2	W		(LE) block data len (Q)
#	+4	B[Q]	block data, see below
#
#	literal block data structure
#	+0	B[Q]	raw data
#
#	packed block data structure
#	+0		B[R]		dictionary 1
#	+B*R	B[R]		dictionary 2
#	+2*B*R	B[R]		dictionary 3
#	+3*B*R	B[Q-3*R]	raw data

import struct
import sys

def expand_packed_block(block, dict_size):
	pos = 0
	dict_fmt = "%iB" % dict_size
	_200 = [0]
	_200.extend( struct.unpack_from(dict_fmt, block, pos) )
	pos += dict_size
	_000 = [0]
	_000.extend( struct.unpack_from(dict_fmt, block, pos) )
	pos += dict_size
	_100 = [0]
	_100.extend( struct.unpack_from(dict_fmt, block, pos) )
	pos += dict_size

	_301 = [0] * 256
	_402 = [0] * (dict_size+1)

	# command list
	cmds = []
	
	# output
	dst = []
	
	for i in range(0, dict_size):
		al = _200[i + 1]
		_402[i + 1] = _301[al]
		_301[al] = i + 1		# use _301 as an array of indexes into _402
	
	dx = len(block) - pos
	done = False
	while (dx > 0):
		dx -= 1
		done = False
	
		al = ord(block[pos])
#		print "byte: %02x" % al
		pos += 1
		
		bl = al
		if _301[bl] == 0:
#			print "straight append: %02x" % al
			dst.append( al )
			continue
		
		bl = _301[bl]
		cmds.append( (0, 0) )
		cmds.append( ( bl, _100[bl] ) )
#		print "START ", cmds
		ah = 0
		al = _000[bl]
	
		# loc_10270
		while not done:
			bp = (ah, al)
			if _301[bp[1]] == 0:
#				print "1st level append: %02x" % al
				dst.append( al )
				(ah, al) = cmds.pop()
#				print "POP  ", cmds
				if (ah, al) == (0, 0):
					break
		
				bl = ah
				ah = 0
				continue
				
			elif bl > _301[bp[1]]:
				bl = _301[bp[1]]
				cmds.append( (bl, _100[bl]) )
#				print "PUSH ", cmds
				ah = 0
				al = _000[bl]
				continue
		
			al = bl
			bl = _301[bp[1]]
		
			while True:
				bl = _402[bl]
				if bl == 0:
					(ah, al) = bp
#					print "2nd level append: %02x" % al
					dst.append( al )
					(ah, al) = cmds.pop()
#					print "POP  ", cmds
					if (ah, al) == (0, 0):
						done = True
						break
		
					bl = ah
					ah = 0
					break	# loc_10270
		
				if bl < al:
					cmds.append( (bl, _100[bl]) )
#					print "PUSH ", cmds
					ah = 0
					al = _000[bl]
					break	# loc_10270			

	return "".join([chr(x) for x in dst])


def expand_blocks(src):
	pos = 0
	dst = ""
	cont_flag = True	# to enter the loop

	while cont_flag:
		(dict_size, cont_flag, block_len) = struct.unpack_from("<BBH", src, pos)
		pos += 4
		
		if dict_size == 0:
			# literal block
#			print "%06x [LIT]: %06x (%06i)" % (pos, block_len, block_len)

			block_data = src[pos:pos+block_len]
			pos += block_len
			dst += block_data
		else:
			# packed block
			full_block_len = dict_size * 3 + block_len
			block_data = src[pos:pos+full_block_len]
			expanded_block_data = expand_packed_block(block_data, dict_size)
#			print "%06x [PAK]: %06x (%06i) -> %06i" % (pos, block_len, block_len, len(expanded_block_data))		
			pos += full_block_len
			dst += expanded_block_data
		
	return dst


data = open(sys.argv[1], "r").read()
num_resources, = struct.unpack_from(">H", data, 0)
header_size = 2 + 4 * num_resources
resource_offsets = [header_size + struct.unpack_from(">I", data, p)[0] for p in range(2, header_size, 4)]

for i, offset in enumerate(resource_offsets):	
	filename = "%s.%03i" % (sys.argv[1], i)

	(packed_size, unpacked_size) = struct.unpack_from(">II", data, offset)
	pos = offset + 8
	unpacked_resource = expand_blocks(data[pos:pos + packed_size])
	print "%s (packed: %i, expected: %i, unpacked: %i)" % (filename, packed_size, unpacked_size, len(unpacked_resource) )

	assert(len(unpacked_resource) == unpacked_size)
	open(filename, "w").write(unpacked_resource)
