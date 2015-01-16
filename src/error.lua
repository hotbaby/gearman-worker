--Error massage table

ErrorMsgTable = {}

--general error code
ErrorMsgTable[0000] = "success"
ErrorMsgTable[1000] = "not implement"
ErrorMsgTable[1001] = "param error"
ErrorMsgTable[1002] = "server internal error"

--wan management error code
ErrorMsgTable[4001] = "interface not exit"
ErrorMsgTable[4002] = "protocol error"
ErrorMsgTable[4003] = "ip addr error"
ErrorMsgTable[4004] = "netmask error"
ErrorMsgTable[4005] = "PPPoE username or passwd error"
ErrorMsgTable[4006] = "VPN username or passwd error"

--wifi management error code
ErrorMsgTable[4101] = "unsupported band type"
ErrorMsgTable[4102] = "unsupported encryption type"
ErrorMsgTable[4103] = "channel error"
ErrorMsgTable[4104] = "htmode error"

--wds management error code
ErrorMsgTable[4201] = "scan error"
ErrorMsgTable[4202] = "connect error"

--router advanced management error code
ErrorMsgTable[4301] = "DHCP default gateway error"
ErrorMsgTable[4302] = "DNS name server error"
ErrorMsgTable[4303] = "DDNS domain error"
ErrorMsgTable[4304] = "DDNS username or passwd error"
ErrorMsgTable[4305] = "LAN is not correctly set"
ErrorMsgTable[4306] = "WAN is not correctly set"

--mac filter management error code
ErrorMsgTable[4401] = "white record already exited"
ErrorMsgTable[4402] = "White record not exit"
ErrorMsgTable[4403] = "Black record already exited"
ErrorMsgTable[4404] = "Black record not exit"

--internet access control management error code
ErrorMsgTable[4501] = "filter mac addr error"
ErrorMsgTable[4502] = "filter record already exited"
ErrorMsgTable[4503] = "filter record not exit"

--qos management error code
ErrorMsgTable[4601] = "unsupported priority type"

--advance error cod
ErrorMsgTable[4701] = "static dns not exit"

return ErrorMsgTable
