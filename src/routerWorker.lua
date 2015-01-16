#! /usr/bin/env lua
local worker = require("worker")
local cjson  = require("cjson")
local wan    = require("wan")

--register modules
local moduleTable = {}
moduleTable[wan.name]    = wan

--suspended task list
local suspendedTaskList = {}


function getMoudleAndFunc(method)
    local module
    local func

    if type(method) ~= "string" then
        return nil
    end

    local s, e
    s, e = string.find(method, "%a*")
    module = string.sub(method, s, e)

    s, e = string.find(method, "%.%a*")  --split module and function
    func = string.sub(method, s+1, e)

    return module, func
end

function dispatch(jsonObj, t)
    local module, func
    local method, params, id
    local errorCode, result, errorMsg
    local suspendedFunc

    if type(jsonObj) ~= "table" or type(t) ~= "table" then
        print("dispatch: params error.")
        return nil
    end

    method = jsonObj['method']
    params = jsonObj['params']
    id     = jsonObj['id']

    module, func = getMoudleAndFunc(method)

    if t[module] and t[module][func] then
        errorCode, result, suspendedFunc = t[module][func](unpack(params))
    else
        print("module or func not exit.")
        errorCode = 1000
    end

    if suspendedFunc ~= nil and type(suspendedFunc) == "function" then
        table.insert(suspendedTaskList, suspendedFunc)
    end

    local jsonResponseMessage
    local errorTable = require "error"

    if errorCode == nil then
        if result == nil then
            result = true
        end
        errorMsg = nil
    else
        errorMsg = {}
        errorMsg['error_code'] = errorCode
        if not errorTable[errorCode] then
            print("error code " .. errorCode .. " not exit.")
            errorMsg['error_msg'] = nil
        else
            errorMsg['error_msg'] = errorTable[errorCode]
        end
    end

    jsonResponseMessage = cjson.encode({
        ['result'] = result,
        ['error']  = errorMsg,
        ['id'] = id,
    })

    return jsonResponseMessage
end

function callBack(size, buffer)
    local jsonResponseMessage
    local jsonRequestMessage = buffer
    local time1, time2

    time1 = os.clock()
    print("request time:" .. os.date())
    print("request message:" .. buffer)

    jsonRequestObj = cjson.decode(jsonRequestMessage)
    jsonResponseMessage = dispatch(jsonRequestObj, moduleTable)

    print("response message:" .. jsonResponseMessage)
    print("response time:" .. os.date())
    time2 = os.clock()
    print("diff time:" .. os.difftime(time2, time1) .. "s")

    return jsonResponseMessage
end

suspendedTaskRoutine = coroutine.create(function ()
    while true do
        print("call suspended tasks coroutine.")
        print("suspendedTaskList length:" .. #suspendedTaskList)
        local fn
        if #suspendedTaskList > 0 then
            for k,v in pairs(suspendedTaskList) do
                fn = suspendedTaskList[k]
                ret = type(fn) == "function" and fn()
            end
        end
        suspendedTaskList = {}
        coroutine.yield()
    end
end)

workerRoutine = coroutine.create(function ()
        routerWorker = worker.initialize("127.0.0.1", 4730, false, 0, "router")

    while true do
        routerWorker:loop(callBack)
        if #suspendedTaskList > 0 then
            common.sleep(1)
            coroutine.resume(suspendedTaskRoutine)
        end
    end
end)

coroutine.resume(workerRoutine)
