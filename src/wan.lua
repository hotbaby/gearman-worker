--WAN config package

WAN = {}
WAN.name = "WAN"

function WAN.setConnectInfo(...)
    local errorCode
    local result
    args = { ... }

    print("call function WAN.setConnectInfo.")

    local protocol = args[1]
    if protocol == 'pppoe' then
        local info = args[2]
        local username = info['username']
        local password = info['password']

        print('protocol:'.. protocol)
        print('detail info:')
        print('\t username:', username)
        print('\t password:', password)

        errorCode = nil
        result = true

        return errorCode, result
    else
        errorCode = 1001
        return errorCode
    end


end

local mt = {
    __index = function(WAN, method)
        error("method:" .. method .. " not exit.")
    end,
    __newindex = function(WAN, method, newFunc)
        error("update of table " .. tostring(method) .. " " .. tostring(newFunc))
    end
}
setmetatable(WAN, mt)

return WAN
