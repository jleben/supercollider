ScopeBuffer {
	var <index, <numChannels, <size, <server;

	*alloc { arg server, numChannels=1, size=4096;
		var index;
		server = server ? Server.default;
		index = server.scopeBufferAllocator.alloc(numChannels);
		if (index.isNil) {
			error("Meta_ScopeBuffer:alloc: failed to get a scope buffer allocated."
				+ "numChannels:" + numChannels + "server:" + server.name);
			^nil
		};
		server.sendMsg( "/cmd", \allocateScopeBuffer, index, numChannels, size );
		^this.new(index, numChannels, size, server)
	}

	*new { arg index=0, numChannels=1, size=4069, server;
		^super.newCopyArgs(index, numChannels, size, server ? Server.default)
	}

	// compatibility method
	bufnum {
		^index;
	}

	free {
		if (index.isNil) {
			(this.asString + " has already been freed").warn;
			^this
		};

		server.scopeBufferAllocator.free(index);
		server.sendMsg( "/cmd", \releaseScopeBuffer, index );
		index = nil;
		numChannels = nil;
		size = nil;
	}

	printOn { arg stream;
		stream << this.class.name << "(" <<*
			[index, numChannels, size, server]  <<")"
	}

	storeOn { arg stream;
		stream << this.class.name << "(" <<*
			[index, numChannels, size, server.asCompileString]  <<")"
	}

	== { arg that;
		^this.compareObject(that, #[\index, \numChannels, \size, \server])
	}

	hash {
		^this.instVarHash(#[\index, \numChannels, \size, \server])
	}

}
