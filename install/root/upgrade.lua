#!/usr/bin/env lua

VERSION="0.1.5"
COPY="Copyright (C) 2013 Lierda Science& Technology Group Co., Ltd"

--device type
DEV_TYPE = {
    NONE = -1,
    LIGHT = 0,
    CURTAIN = 1,
    METER = 2,
    A = 3,
    B = 4,
    D = 5
}

DEV_TYPE_NAME = {
    [0] = "灯控器",
    [1] = "帘控器",
    [2] = "抄表接口",
    [3] = "A",
    [4] = "B",
    [5] = "D"
}

SERIALPORT="/dev/ttyUSB0"
g_upgrade_file = nil
g_is_flash_directly = false
g_is_auto_upgrade = false
g_is_check_subtype = true

-- turn off io buffer, remote ssh call will mess
io.stdout:setvbuf 'no'

function help(program)
    print(string.format("版本:%s %s", VERSION, COPY))
    print("用法:")
    print(string.format("  %s [升级文件] [选项]", program))
    print("    -f    直接Bootloader升级")
    print("    -a    自动升级当前目录下的所有*.bin文件")
    print("    -n    不要检查设备子类型(ABD)型灯控器或帘控器")
    print("    -v    版本")
end

function check_file_type(file)
    local file_type = nil
    local file_subtype = nil
    if string.find(file, "LampCtrl") then
        file_type = DEV_TYPE.LIGHT
    elseif string.find(file, "CurtainCtrl") then
        file_type = DEV_TYPE.CURTAIN
    elseif string.find(file, "mb9bf321k") then
        file_type = DEV_TYPE.METER
    else
        return nil, nil
    end

    if file_type == DEV_TYPE.LIGHT or file_type == DEV_TYPE.CURTAIN then
        if string.find(file, "AAA") then
            file_subtype = DEV_TYPE.A
        elseif string.find(file, "BBB") then
            file_subtype = DEV_TYPE.B
        elseif string.find(file, "DDD") then
            file_subtype = DEV_TYPE.D
        end
    end
    return file_type, file_subtype
end

function get_device_subtype(dev_type, addr)
    local dev_subtype = nil
    local opt = nil
    local tmpfile = "/tmp/tmp.upgrade.devs"

    if dev_type == DEV_TYPE.LIGHT then
        opt = "-S -l "..addr
    elseif dev_type == DEV_TYPE.CURTAIN then
        opt = "-S -c "..addr
    elseif dev_type == DEV_TYPE.METER then
        opt = "-S -m "..addr
    end

    assert(opt)
    assert(SERIALPORT)

    local ret = os.execute(string.format("/root/server485 %s %s >%s", opt, SERIALPORT, tmpfile))
    if ret == 0 then
        local f = io.open(tmpfile)
        if f ~= nil then
            local all_value = f:read("*all")
            if string.find(all_value, "A") then
                dev_subtype = DEV_TYPE.A
            elseif string.find(all_value, "B") then
                dev_subtype = DEV_TYPE.B
            elseif string.find(all_value, "D") then
                dev_subtype = DEV_TYPE.D
            else
                io.stderr:write("ERROR: tmpfile里找不到ABD\n")
            end
            f:close()
        else
            io.stderr:write(string.format("ERROR: 打开tmpfile %s失败\n", tmpfile))
        end
    else
        io.stderr:write("WARNNING: 发送获取设备子类型命令失败\n")
    end
    return dev_subtype
end

function cmd_setup_mode(dev_type, addr)
    local opt = nil
    if dev_type == DEV_TYPE.LIGHT then
        opt = "-l "..addr
    elseif dev_type == DEV_TYPE.CURTAIN then
        opt = "-c "..addr
    elseif dev_type == DEV_TYPE.METER then
        opt = "-m "..addr
    end

    assert(opt)
    assert(SERIALPORT)

    local ret = os.execute(string.format("/root/server485 %s %s >/dev/null", opt, SERIALPORT))
    if ret == 0 then
        return true
    else
        return false
    end
end

function scan_device()
    local devs_table = {}
    local str_error = ""
    local tmpfile = "/tmp/tmp.scan.devs"
    local dev_count = 0
    print("正在扫描设备...")
    assert(SERIALPORT)
    local ret = os.execute(string.format("/root/server485 -S %s >%s", SERIALPORT, tmpfile))
    if ret == 0 then
        local f = io.open(tmpfile)
        if f ~= nil then
            while true do
                local line_value = f:read("*line")
                if not line_value then
                    break
                end
                local ret, str_type, str_addr
                ret, _, str_type, str_addr = string.find(line_value, "(%S+):%s+([%d%s]+)")
                if ret then
                    local dev_type = nil
                    local subtype_list = {}

                    if str_type == "light" then
                        dev_type = DEV_TYPE.LIGHT
                    elseif str_type == "curtain" then
                        dev_type = DEV_TYPE.CURTAIN
                    elseif str_type == "meter" then
                        dev_type = DEV_TYPE.METER
                    else
                        -- 不支持的设备 dev_type = nil
                        --assert(false)
                    end

                    print(DEV_TYPE_NAME[dev_type], str_addr)

                    for _addr in string.gmatch(str_addr, "%d+") do
                        local addr = tonumber(_addr)
                        assert(addr)

                        if dev_type == DEV_TYPE.LIGHT or dev_type == DEV_TYPE.CURTAIN then
                            local subtype = get_device_subtype(dev_type, addr)
                            if not subtype then
                                io.stderr:write(string.format("ERROR: 查询子设备类型失败, 类型:%d, 485地址:%d\n", dev_type, addr))
                                str_error = str_error .. string.format("%s 485地址:%d 旧固件不支持自动升级\n", DEV_TYPE_NAME[dev_type], addr)
                            else
                                -- 加进子设备类型表
                                subtype_list[addr] = subtype
                            end
                        else
                            -- 加进子设备类型表
                            subtype_list[addr] = DEV_TYPE.NONE
                        end

                        dev_count = dev_count + 1
                    end

                    -- 加进设备类型表
                    if dev_type then
                        devs_table[dev_type] = subtype_list
                    end
                end
            end
            f:close()
            return true, devs_table, dev_count, str_error
        else
            io.stderr:write(string.format("ERROR: 打开tmpfile %s失败\n", tmpfile))
        end
    end
    return false, devs_table, 0, str_error
end
        
function send_file(file, file_type)
    assert(file_type)
    --stty -F $dev 9600 raw parenb -parodd cs8 -cstopb
    local ret = 0
    ret=os.execute("./ymodem_send -t -d "..SERIALPORT)
    if ret ~= 0 then
        io.stderr:write(string.format("ERROR: ymodem_send -t -d %s fail!!!\n", SERIALPORT))
        return false
    end

    os.execute("sleep 1")
    os.execute("echo -n c >"..SERIALPORT)
    os.execute("sleep 1")
    os.execute("echo -n 1 >"..SERIALPORT)
    os.execute("sleep 1")
    ret = os.execute(string.format("./ymodem_send -d %s '%s'", SERIALPORT, file))
    if ret ~= 0 then
        io.stderr:write(string.format("ERROR: ymodem_send -d %s %s fail!!!\n", SERIALPORT, file))
        return false
    end

    os.execute("sleep 1")
    if file_type == DEV_TYPE.METER then
        --meter is 5, other is 4
        print("抄表接口升级")
        os.execute("echo -n 5 >"..SERIALPORT)
    else
        os.execute("echo -n 4 >"..SERIALPORT)
    end
    return true
end

function main()
    if arg[1] == nil or arg[2] ~= nil and arg[2] ~= "-f" and arg[2] ~= "-n" then
        help(arg[0])
        return false
    end

    if arg[1] == "-a" then
        g_is_auto_upgrade = true
    elseif arg[1] == "-v" then
        print(VERSION)
        return true
    else
        g_upgrade_file = arg[1]
        if arg[2] ~= nil then
            if arg[2] == "-f" then
                g_is_flash_directly = true
            elseif arg[2] == "-n" then
                g_is_check_subtype = false
            end
        end
    end

    -- init check
    local pid = io.open("/proc/self/stat"):read("*number")
    if pid ~= nil then
        local is_running = os.execute(string.format("ps | grep upgrade.lua | grep -v grep | grep -v ash | grep -v %d >/dev/null", pid))
        if is_running == 0 then
            io.stderr:write("ERROR: 重复运行升级\n")
            return false
        end
    else
        io.stderr:write("WARNNING: 读取/proc/self/stat失败\n")
    end
    os.execute("/etc/init.d/xmlrpc stop")

    -- check firmware file
    local file_cur_type = nil
    local file_cur_subtype = nil
    if g_upgrade_file ~= nil then
        file_cur_type, file_cur_subtype = check_file_type(g_upgrade_file)
        if file_cur_type == nil then
            io.stderr:write("ERROR: 无效固件文件名 "..g_upgrade_file.."\n")
            return false
        end
    end

    if g_is_flash_directly then
        assert(SERIALPORT)
        if send_file(g_upgrade_file, file_cur_type) then
            print("进Bootloader升级成功")
            return true
        else
            print("进Bootloader升级失败")
            return false
        end
    end

    if g_is_auto_upgrade then
        -- 自动升级
        -- get dir file lists
        local file_type_list = {}
        local file_lists = io.popen("ls -1 /tmp/*.bin")
        local file_counts = 0

        print("*************************************************************")
        while true do
            local line_value = file_lists:read("*line")
            if not line_value then
                break
            end

            local file_type, file_subtype
            file_type, file_subtype = check_file_type(line_value)
            --print(line_value, file_type, file_subtype)

            --file_type_list[file_type][file_subtype] == firwarefilename
            if file_type and file_type_list[file_type] == nil then
                file_type_list[file_type] = {}
            end

            if file_type and file_type_list[file_type][file_subtype] == nil then
                if file_subtype then
                    file_type_list[file_type][file_subtype] = line_value
                else
                    file_type_list[file_type][DEV_TYPE.NONE] = line_value
                end
                file_counts = file_counts + 1
            else
                io.stderr:write("ERROR: 有重复或无效的固件\n")
                return false
            end
        end
        file_lists:close()

        if file_counts == 0 then
            io.stderr:write("ERROR: 没有找到可以升级的固件\n")
            return false
        end

        local ret, dev_table, dev_count, str_error
        ret, dev_table, dev_count, str_error = scan_device()
        if not ret then
            return false
        elseif dev_count == 0 then
            io.stderr:write(string.format("ERROR: 扫描到0个设备\n%s",str_error))
            return false
        else
            print(string.format("扫描到%d个设备", dev_count))
        end

        -- 按每个固件搜索对应的设备
        local file_dev_type, file_subtype, firmware_file, v
        local upgrade_count = 0
        for file_dev_type, v in pairs(file_type_list) do
            for file_subtype, firmware_file in pairs(v) do
                print("-------------------------------------------------------------")
                print("固件:", firmware_file)
                if dev_table[file_dev_type] ~= nil then
                    local addr, subtype
                    for addr, subtype in pairs(dev_table[file_dev_type]) do
                        if subtype == file_subtype then
                            -- 全部匹配，开始升级
                            print("自动升级", "设备:", DEV_TYPE_NAME[file_dev_type], "地址:", addr)
                            os.execute("sleep 1")
                            local ret = cmd_setup_mode(file_dev_type, addr)
                            if not ret then
                                io.stderr:write("ERROR: 设备进入升级模式失败\n")
                                print("自动升级失败, 失败固件:", firmware_file, "addr:", addr)
                                return false
                            end

                            ret = send_file(firmware_file, file_dev_type)
                            if ret then
                                print("升级成功")
                                upgrade_count = upgrade_count + 1
                            else
                                print("升级失败, 尝试进Bootloader烧写...")
                                os.execute("sleep 3")
                                if send_file(firmware_file, file_dev_type) then
                                    print("升级成功")
                                    upgrade_count = upgrade_count + 1
                                else
                                    print("自动升级失败, 失败固件:", firmware_file, "addr:", addr)
                                    return false
                                end
                            end
                        end
                    end
                end
            end
        end

        if upgrade_count == 0 then
            io.stderr:write(string.format("ERROR: 升级0个设备\n%s", str_error))
            return false
        else
            print(string.format("升级了%d个设备\n%s", upgrade_count, str_error))
        end
    else
        -- upgrade the firmware file
        io.write("输入485地址:")
        local addr = io.read()
        addr = tonumber(addr)
        if addr and addr >= 1 and addr <= 8 then

            -- check device subtype
            if g_is_check_subtype and file_cur_subtype then
                local cur_subtype = get_device_subtype(file_cur_type, addr)
                if cur_subtype then
                    print(string.format("固件子类型 %d, 设备子类型 %d", file_cur_subtype, cur_subtype))
                    if cur_subtype and file_cur_subtype == cur_subtype then
                        print("类型匹配")
                    else
                        io.stderr:write("ERROR: 设备子类型不符合\n")
                        return false
                    end
                else
                    io.stderr:write("ERROR: 无设备子类型，无法升级\n")
                    return false
                end
            end

            local ret = cmd_setup_mode(file_cur_type, addr)
            if not ret then
                io.stderr:write("ERROR: 设备进入升级模式失败\n")
                return false
            end

            assert(SERIALPORT)
            ret = send_file(g_upgrade_file, file_cur_type)
            if ret then
                print("升级成功")
                return true
            else
                print("升级失败, 尝试进Bootloader烧写...")
                os.execute("sleep 3")
                if send_file(g_upgrade_file, file_cur_type) then
                    print("升级成功")
                    return true
                else
                    print("升级失败")
                    return false
                end
            end
        else
            io.stderr:write("ERROR: 无效485地址\n")
            return false
        end
    end
    return true
end

os.exit((main() and 0) or 1)
