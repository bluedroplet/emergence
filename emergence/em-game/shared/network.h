

#define EMNETINDEX_MAX					0x07ffffff
#define EMNETINDEX_MASK					EMNETINDEX_MAX
#define EMNETCOOKIE_MASK				EMNETINDEX_MASK



#define EMNETCLASS_MASK				(0x3 << 30)
#define EMNETFLAG_MASK					(0x7 << 27)


#define EMNETCLASS_CONTROL			(0x0 << 30)

#define EMNETFLAG_CONNECT				(0x0 << 27)		// sent by client to start connection handshake
														// and subsequently returned by server after cookie verification
#define EMNETFLAG_COOKIE				(0x1 << 27)		// sent by server
														// and subsequently returned by client
#define EMNETFLAG_DISCONNECT			(0x2 << 27)		// sent by either server or client
#define EMNETFLAG_ACKWLDGEMNT			(0x3 << 27)		// acknowledgement of EMNETFLAG_STREAMDATA* or EMNETFLAG_DISCONNECT


#define EMNETCLASS_STREAM			(0x1 << 30)		// sent by either server or client

#define EMNETFLAG_STREAMFIRST			(0x1 << 27)
#define EMNETFLAG_STREAMLAST			(0x2 << 27)
#define EMNETFLAG_STREAMRESNT			(0x4 << 27)


#define EMNETCLASS_MISC				(0x3 << 30)

#define EMNETFLAG_PING					(0x0 << 27)		// with cookie
#define EMNETFLAG_PONG					(0x1 << 27)		// with cookie

#define EMNETFLAG_SERVERINFO			(0x7 << 27)		// info about server


#define EMNETCONNECT_MAGIC				0x02708d11



#define EMNETPACKET_SIZE				512
#define EMNETHEADER_SIZE				4
#define EMNETPAYLOAD_SIZE				508






struct packet_t
{
	uint32_t header;
	uint8_t payload[EMNETPAYLOAD_SIZE];
};




#define NETWORK_PORT			45420


#define EMNETMSG_PRINT			1

#define EMNETMSG_PROTO_VER		0
#define EMNET_PROTO_VER			1



// server -> client

#define EMNETMSG_PLAYING		2
#define EMNETMSG_SPECTATING		3


#define EMNETMSG_INRCON			4
#define EMNETMSG_NOTINRCON		5
#define EMNETMSG_JOINED			6
#define EMNETMSG_EVENT			7
#define EMNETMSG_LOADMAP		8
#define EMNETMSG_LOADSKIN		9

#define EMNETMSG_PING			40
#define EMNETMSG_PONG			41

#define EMNETEVENT_SPAWN_ENT	0
#define EMNETEVENT_UPDATE_ENT	1
#define EMNETEVENT_KILL_ENT		2
#define EMNETEVENT_FOLLOW_ME	3
#define EMNETEVENT_CARCASS		4
#define EMNETEVENT_RAILTRAIL	5
#define EMNETEVENT_DUMMY		20



// client -> server

#define EMNETMSG_JOIN			0
#define EMNETMSG_PLAY			1
#define EMNETMSG_SPECTATE		2
#define EMNETMSG_SAY			3

#define EMNETMSG_NAMECNGE		4
#define EMNETMSG_CTRLCNGE		5
#define EMNETMSG_STATUS			6
#define EMNETMSG_ENTERRCON		7
#define EMNETMSG_LEAVERCON		8
#define EMNETMSG_RCONMSG		9	
#define EMNETMSG_FIRERAIL		11
#define EMNETMSG_FIRELEFT		12
#define EMNETMSG_FIRERIGHT		13
#define EMNETMSG_DROPMINE		14

//#define EMNETMSG_PING			40
//#define EMNETMSG_PONG			41
