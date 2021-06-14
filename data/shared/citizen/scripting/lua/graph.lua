--[[
    A graphing and formatting utility script for the lmprof 'graph' output.

    This script introduces two classes:
        1. LMNode - A graph node that represents a single function instance or
            the accumulation of all instances of the same function.

        2. LMGraph - A graph composed of LMNode instances, measurements about
            them, and functions for generating human readable and application
            specific outputs.

    Three output formats are currently supported:
        1. Flat - A tabular representation that may optionally be delimited, e.g., a CSV.
        2. PepperFish - A layout similar to lua-users.org/wiki/PepperfishProfiler
        3. Callgrind - A callgrind/kcachegrind compatible graph representation.

    The 'header' table of the lmprof 'graph' output supports the additional
    fields related to formatting and output of the graph:
        [BOOLEAN]
        csv - "Flat" layout is comma delimited [BOOLEAN]
        show_lines - Show line frequencies for flat output

        [STRING]
        sort - Sort the LMGraph output by a given category:
            'count',
            'time',
            'total_time',
            'allocated',
            'total_allocated'

@NOTES
    Instead of using io.write, or the 'io' library in general, a simplified
    writer API is used. For example, stdout:

        StdOut = {
            flush = function() end,
            write_line = function(_, str) print(str) end,
        }

    The final argument to each 'format' method will the writer object. Defaulting
    to StdOut on nil.

@EXAMPLE
    -- local result = ... -- some lmprof operation.
    result.header.sort = "count" -- Append additional graphing properties
    result.header.csv = false

    -- Create a new format instance and produce a graph structure from the
    -- result table.
    local graph = Citizen.Graph(result.header, result.records)

    -- Generate a Flat representation
    graph:Flat(result.header)

    -- Generate a Pepperfish representation
    graph:Pepperfish(result.header)

    -- Generate a Callgrind representation
    graph:Callgrind(result.header)

@LICENSE
    See Copyright Notice in lmprof_lib.h
--]]
local LMGraph = nil
local LMNode = nil
local Binary = nil
local Decimal = nil

---------------------------------------
---------- Utility Functions ----------
---------------------------------------

-- Attempt to use the UTF-8 representation of mu, otherwise fallback to u.
local mu = (utf8 and utf8.char(0x03BC)) or "u"

-- When aligning columns use the utf8 length when possible.
local utf8_len = (utf8 and utf8.len) or string.len

--[[
    Formatting labels, If "_VERSION <= Lua 5.2", then the UTF-8 code must be
    replaced with "u".
--]]
local MetrixPrefix = { "n", mu, "m", "", "k", "M", "G", "T", "P", "E", "Z", "Y" }

--[[ Pad a (UTF8) string with left or right justification. --]]
local function string_pad(str, width, padStr)
    local padStr = string.rep(padStr or ' ', math.abs(width))
    local width = width - (utf8_len(str) - str:len())
    return (width < 0 and string.sub(padStr .. str, width))
        or string.sub(str .. padStr, 1, width)
end

--[[ Check a string prefix --]]
local function string_starts(str, start)
   return str:sub(1, #start) == start
end

--[[ Binary (base 2) representation. --]]
Binary = {
    Exp = function(value, degree) return value * (1024 ^ -degree) end,
    Log = function(value)
        if value == 0 then return 0 end
        return math.floor((math.log(math.abs(value)) / math.log(2)) / 10)
    end,

    -- Format the given value according to a fixed degree (see Exp)
    Format = function(value, degree)
        return Binary.MetricFormat(Binary.Exp(value, degree))
    end,

    -- Create a human readable column header.
    Readable = function(degree, offset, suffix)
        return ("%s%s"):format(MetrixPrefix[degree + 1 + offset], suffix or "")
    end,

    -- Return a format string that limits the number of decimal values based on
    -- the size of the value.
    MetricFormat = function(value)
        local formatStr = "%.4f"
        if value > 1000 then
            formatStr = "%.2f"
        elseif value > 100 then
            formatStr = "%.3f"
        elseif value > 10 then
            formatStr = "%.4f"
        end
        return string.format(formatStr, value)
    end
}

--[[ Decimal (base 10) representation --]]
Decimal = {
    Exp = function(value, degree) return value * (1000 ^ -degree) end,
    Log = function(value)
        if value == 0 then return 0 end
        return math.floor((math.log(math.abs(value)) / math.log(10)) / 3)
    end,

    -- Format the given value according to a fixed degree (see Exp)
    Format = function(value, degree)
        return Binary.MetricFormat(Decimal.Exp(value, degree))
    end,

    -- Create a human readable column header.
    Readable = function(degree, offset, suffix)
        return ("%s%s"):format(MetrixPrefix[degree + 1 + offset], suffix or "")
    end,
}

----------------------------------------
------------- Profile Node -------------
----------------------------------------

--[[
    A function node that represents:

    (1) A single 'aggregated' activation record instance, e.g., a <func, parent>
        tuple or <func, parent, parent_line> triple.

    (2) The accumulated statistics of all instances of the same function, its
        sources of invocation (parents), and decadents (children).
--]]
LMNode = setmetatable({
    --[[
        Fields specified by lmprof_report.h mapped to categories, labels, and
        functions:

        Label - A human readable name.
        Sort - A (total) ordering function.
        Cat - Category for post processing: Specifying the unit of measurement,
            how values are aggregated, and their format.
    --]]
    Fields = {
        name = { Label = "Name", Cat = "String", Sort = function(fa, fb) if fa.name == fb.name then return 0 end ; return (fa.name < fb.name and -1) or 1 end },
        count = { Label = "Count", Cat = "Count", Sort = function(fa, fb) return fa.count - fb.count end },
        time = { Label = "Self-Time", Cat = "Time", Sort = function(fa, fb) return fa.time - fb.time end },
        total_time = { Label = "Total-Time", Cat = "Time", Sort = function(fa, fb) return fa.total_time - fb.total_time end },
        allocated = { Label = "Alloc", Cat = "Binary", Sort = function(fa, fb) return fa.allocated - fb.allocated end },
        total_allocated = { Label = "Total", Cat = "Binary", Sort = function(fa, fb) return fa.total_allocated - fb.total_allocated end },
        deallocated = { Label = "Dealloc", Cat = "Binary" },
        total_deallocated = { Label = "Dealloc", Cat = "Binary" },
        lines = { Label = "Lines", Cat = "Table" },
        id = { Label = "ID", Cat = "String" },
        parent = { Label = "PID", Cat = "String" },
        depth = { Label = "Depth", Cat = "Count", Sort = function(fa, fb) return fa.depth - fb.depth end },

        -- Extended fields
        timePerCount = { Label = "Time/Call", Cat = "TimeAverage" },
        totalTimePerCount = { Label = "Total/Call", Cat = "TimeAverage" },
        allocPerCount = { Label = "Alloc/Call", Cat = "BinaryAverage" },
        totalAllocPerCount = { Label = "Total/Call", Cat = "BinaryAverage" },
        timePercent = { Label = "PctTotal", Cat = "Percent" },
        allocPercent = { Label = "PctTotal", Cat = "Percent" },
        childTimePercent = { Label = "Pct Time Child", Cat = "Percent" },
        childSizePercent = { Label = "Pct Alloc Child", Cat = "Percent" },
    },
}, {
    __tostring = function() return "Node" end,
    __call = function(_, ...)
        return LMNode.New(...)
    end
})
LMNode.__index = LMNode

--[[ Create a new function record.  --]]
function LMNode.New(id, record, previous)
    local result = previous or setmetatable({
        id = id,
        record = record,
        name = (record == nil and "") or record.source,
        -- Statistics
        count = 0,
        child_count = 0, -- Total number of sampler hits in all descendants
        -- Graph Structure
        rechable = false,
        depth = math.huge, -- Distance from root
        parents = { },
        children = { },
    }, LMNode)

    -- Ensure a record exists.
    if result.record == nil then
        result.record = record
    end

    -- Time profiling statistics
    if record ~= nil and record.time ~= nil then
        result.time = 0.0
        result.total_time = 0.0
        result.timePerCount = 0.0
        result.totalTimePerCount = 0.0
        result.timePercent = 0.0
        result.childTimePercent = 0.0
    end

    -- Memory profiling statistics
    if record ~= nil and record.allocated ~= nil then
        result.allocated = 0.0
        result.deallocated = 0.0
        result.total_allocated = 0.0
        result.total_deallocated = 0.0
        result.allocPerCount = 0.0
        result.totalAllocPerCount = 0.0
        result.allocPercent = 0.0
        result.childSizePercent = 0.0
    end

    -- Line Frequency
    if record ~= nil and record.lines ~= nil then
        result.lines = { }
    end

    return result
end

--[[
    Merge the profile statistics of 'record' into this node, while associating
    an additional parent node.

    In other words, the 'parent' function invoked 'this' function and had
    accumulated a set amount of statistics.
--]]
function LMNode:Append(header, record, parentNode)
    local tcp = "%(%.%.%.tail calls%.%.%.%)"
    local name = self.name
    local r_name = record.source
    local r_id = (header.compress_graph and record.func) or record.id

    -- Ensure parent/child relationship is set.
    if parentNode ~= nil then
        self.parents[record.parent] = parentNode
        parentNode.children[r_id] = self
    end

    -- always try to use the function name instead of (...tail calls...)
    if (name:match(tcp) and not r_name:match(tcp))
        or (string_starts(name, "?") and not string_starts(r_name, "?"))
        or (not r_name:match(tcp) and utf8_len(r_name) > utf8_len(name)) then
        self.name = r_name
    end

    -- Update statistics. Do not update the 'total' fields for self calls as
    -- they will have been handled by 'measured_pop'.
    --
    -- @NOTE: When compress_graph is enabled, record.parent is its unique
    --  record identifier, not the function identifier.
    if r_id ~= record.parent then
        for k,_ in pairs(self) do
            local f = LMNode.Fields[k]
            if record[k] and f and (f.Cat == "Count" or f.Cat == "Time" or f.Cat == "Binary") then
                self[k] = self[k] + record[k]
            end
        end
    else
        self.count = self.count + record.count
        if self.time then self.time = self.time + record.time end
        if self.allocated then
            self.allocated = self.allocated + record.allocated
            self.deallocated = self.deallocated + record.deallocated
        end
    end

    -- If line_freq was enabled, combine frequencies
    if record.lines ~= nil then
        local funcLines = self.lines
        for i=1,#record.lines do
            if funcLines[i] ~= nil then
                funcLines[i] = funcLines[i] + record.lines[i]
            else
                funcLines[i] = record.lines[i]
            end
        end
    end
end

--[[
    Depth-first iterate through the reduced graph structure, setting a reachable
    field to each processed node to true.
--]]
function LMNode:DepthFirstReachable(depth)
    if not self.reachable then
        local child_count = 0

        self.depth = depth
        self.reachable = true
        for _,childNode in pairs(self.children) do
            childNode:DepthFirstReachable(depth + 1)
            child_count = child_count + childNode.child_count + childNode.count
        end

        self.child_count = child_count
    else
        self.depth = math.min(self.depth or 0, depth)
    end
end

----------------------------------------
---------------- Reduce ----------------
----------------------------------------

--[[
    A graph composed of LMNode instances, measurements about all nodes in the
    graph, and generating a human readable output.
--]]
LMGraph = setmetatable({
    --[[ IO Interface --]]
    StdOut = {
        flush = function() end,
        write_line = function(_, str)
            Citizen.Trace(str)
            Citizen.Trace("\n")
        end,
    },

    --[[
        Sorting categories: an ordered list of fields ordered by priority. Where
        successive fields are used to handle tie breaks.
    --]]
    Sorting = {
        count = { "count", "depth", "name" },
        time = { "time", "count", "depth", "name"  },
        total_time = { "total_time", "count", "depth", "name"  },
        allocated = { "allocated", "count", "depth", "name" },
        total_allocated = { "total_allocated", "count", "depth", "name" },
    },

}, {
    __tostring = function() return "Profile Graph" end,
    __call = function(_, ...)
        return LMGraph.New(...)
    end
})
LMGraph.__index = LMGraph

--[[ Create a new format graph --]]
function LMGraph.New(header, outputTable)
    local self = setmetatable({
        root = nil,
        functions = { }, -- A 'node' referencing each profiled function.
        global = {
            count = 0, -- Total number of function calls.
            size = 0, -- Total amount of memory allocated,
            time = 0, -- Total execution time
            max_count = 0, -- Large record count
            max_size = 0, -- Largest allocation record
            max_time = 0, -- Largest timing record
        },
    }, LMGraph)

    -- Create function records,: store all records of the same function, or
    -- identifier, in one object.
    for i=1,#outputTable do
        local record = outputTable[i]
        local recordId = (header.compress_graph and record.func) or record.id
        local parentRecordId = record.parent

        local node = self.functions[recordId]
        if not node or node.record == nil then -- Create a node
            node = LMNode.New(recordId, record, node)
            self.functions[recordId] = node
            if record.func == "0" then  -- root of the graph
                self.root = node
            end
        end

        local parentNode = nil
        if record.func ~= "0" then
            parentNode = self.functions[parentRecordId]
            if not parentNode then -- Preallocate the parent node.
                parentNode = LMNode.New(parentRecordId, nil, nil)
                self.functions[parentRecordId] = parentNode
                if record.parent == "0" then
                    self.root = parentNode
                end
            end
        end

        -- Update function statistics
        node:Append(header, record, parentNode)

        -- Update global maximums
        self.global.count = self.global.count + record.count
        self.global.time = self.global.time + ((header.instrument and record.time) or 0)
        self.global.size = self.global.size + ((header.memory and record.allocated) or 0)
    end

    -- Ensure a graph exists.
    if #outputTable == 0 then
        error("Script requires load_stack or reduced instructions count")
    elseif self.root == nil then
        error("Root node/record does not exist!")
    end

    -- Ensure the graph is completely connected, i.e., an undirected path exists
    -- between all funcNodes. ... This script doesn't support multiple subgraphs
    -- at the moment although a trivial extension.
    self.root:DepthFirstReachable(0)
    for id,node in pairs(self.functions) do
        if not node.reachable then
            error(("%s is not reachable!"):format(node.name))
        end

        -- Ensure the child table if each node is properly initialized
        for pid,parentNode in pairs(node.parents) do
            parentNode.children[id] = node
            if not header.compress_graph and not node.parent then
                node.parent = parentNode.id
            end
        end
    end

    -- Calculate relative statistics based on the global accumulated values.
    local global_size = self.global.size
    local global_time = self.global.time
    for id,node in pairs(self.functions) do
        self.global.max_count = math.max(self.global.max_count, node.count)

        if header.instrument and node.record ~= nil and node.time ~= nil then
            self.global.max_time = math.max(self.global.max_time, node.time)

            node.timePercent = 0.0
            node.childTimePercent = 0.0
            node.timePerCount = 0
            node.totalTimePerCount = 0
            if node.count ~= 0 then
                node.timePerCount = node.time / node.count
                node.totalTimePerCount = node.total_time / node.count
            end

            if global_time ~= 0 then
                node.timePercent = 100.0 * (node.time / global_time)
                node.childTimePercent = 100.0 * ((node.total_time - node.time) / global_time)
            end
        end

        if header.memory and node ~= nil and node.record ~= nil and node.allocated ~= nil then
            self.global.max_size = math.max(self.global.max_size, node.allocated or 0)

            node.allocPercent = 0.0
            node.childSizePercent = 0.0
            node.allocPerCount = 0
            node.totalAllocPerCount = 0
            if node.count ~= 0 then
                node.allocPerCount = (node.allocated / node.count)
                node.totalAllocPerCount = (node.total_allocated / node.count)
            end

            if global_size ~= 0 then
                node.allocPercent = 100.0 * (node.allocated / global_size)
                node.childSizePercent = 100.0 * ((node.total_allocated - node.allocated) / global_size)
            end
        end
    end

    return self
end

--[[ Compute the base SI unit for timing. --]]
function LMGraph:CreateTimeUnits(header, max_time, max_count)
    -- lmprof has the ability to change the base measurement of time, parse the
    -- data from the header.
    local timeOffset = 0
    if header.clockid == "micro" or header.clockid == 'Krdtsc' then
        timeOffset = 1
    end

    local timeDegree,timeSuffix,timeUnit = nil,nil,nil
    if header.clockid == "rdtsc" or header.clockid == 'Krdtsc' then
        timeUnit = "Tick"
        timeDegree = Decimal.Log(max_time)
        timeSuffix = Decimal.Readable(timeDegree, timeOffset, timeUnit)
    else
        timeUnit = "s"
        timeDegree = Decimal.Log(max_time)
        timeSuffix = Decimal.Readable(timeDegree, timeOffset, timeUnit)
    end

    local timePerDegree = timeDegree - Decimal.Log(max_count)
    local timePerSuffix = Decimal.Readable(timePerDegree, timeOffset, timeUnit)
    return timeDegree,timeSuffix,timePerDegree,timePerSuffix
end

--[[ Compute the base SI unit for memory usage. --]]
function LMGraph:CreateByteUnits(header, max_size, max_count)
    local byteDegree = Binary.Log(max_size)
    local bytePerDegree = byteDegree - Binary.Log(max_count)
    local byteSuffix = Binary.Readable(byteDegree, 3, "B")
    local bytePerSuffix = Decimal.Readable(bytePerDegree, 3, "B")
    return byteDegree,byteSuffix,bytePerDegree,bytePerSuffix
end

--[[ Output debug statistics of the profile --]]
function LMGraph:Verbose(header, outfile)
    local outfile = outfile or LMGraph.StdOut
    local timeDegree,timeSuffix,_,_ = self:CreateTimeUnits(header, header.profile_overhead, 1)

    local buff = { }
    buff[#buff + 1] = { "Header", "" }
    buff[#buff + 1] = { "Clock ID", tostring(header.clockid)}
    buff[#buff + 1] = { "Factored Profile Overhead", ("%s %s"):format(Decimal.Format(header.profile_overhead, timeDegree), timeSuffix)}
    buff[#buff + 1] = { "Calibration", ("%s %s"):format(Decimal.Format(header.calibration, timeDegree), timeSuffix)}
    buff[#buff + 1] = { "Instruction Counter", tostring(header.instr_count)}
    buff[#buff + 1] = { "", "" }

    if header.callback then -- Append Trace Event statistics
        buff[#buff + 1] = { "Trace Events", "" }
        buff[#buff + 1] = { "Compressed Events", tostring(header.compress) }
        buff[#buff + 1] = { "Compression Threshold", tostring(header.threshold) }
        buff[#buff + 1] = { "Initial Page Size" , tostring(header.pagesize) }
        buff[#buff + 1] = { "Initial Page Limit", tostring(header.pagelimit) }
        buff[#buff + 1] = { "Total Page Count" , tostring(header.totalpages) }
        buff[#buff + 1] = { "Page Count", tostring(header.usedpages) }
        buff[#buff + 1] = { "Page Size" , tostring(header.pagesize) }
        buff[#buff + 1] = { "Events per Page", tostring(header.eventpages) }
        buff[#buff + 1] = { "Buffer Usage", (header.pagelimit > 0 and ("%2.3f"):format(header.pageusage)) or "Inf" }
    end

    if header.debug then -- Debug hashing statistics.
        buff[#buff + 1] = { "Hashing", "" }
        buff[#buff + 1] = { "Buckets" , tostring(header.debug.buckets) }
        buff[#buff + 1] = { "Used Buckets", tostring(header.debug.used_buckets) }
        buff[#buff + 1] = { "Record Count", tostring(header.debug.record_count) }
        buff[#buff + 1] = { "Min Bucket Count", tostring(header.debug.min) }
        buff[#buff + 1] = { "Max Bucket Count", tostring(header.debug.max) }
        buff[#buff + 1] = { "Mean (all)", string.format("%2.3f", header.debug.mean) }
        buff[#buff + 1] = { "Variance (all)", string.format("%2.3f", header.debug.var) }
        buff[#buff + 1] = { "Mean (hits)", string.format("%2.3f", header.debug.mean_hits) }
        buff[#buff + 1] = { "Variance (hits)", string.format("%2.3f", header.debug.var_hits) }
        buff[#buff + 1] = { "", "" }
    end

    local longestLabel = 0
    for i=1,#buff do longestLabel = math.max(utf8_len(buff[i][1]), longestLabel) end
    for i=1,#buff do
        outfile:write_line(("%s %s"):format(string_pad(buff[i][1], longestLabel + 1), buff[i][2] or ""))
    end
end

--[[ Create a table.sort function for ordering funcNodes (NewFunctionRecord). --]]
local function CreatePrioritySort(self, priority)
    priority = priority or LMGraph.Sorting["count"]
    return function(a, b)
        local fa,fb = self.functions[a],self.functions[b]
        for i=1,#priority do
            local sort = LMNode.Fields[priority[i]].Sort
            local comparator = sort(fa, fb)
            if comparator ~= 0 then
                return comparator > 0
            end
        end
        return fa.name > fb.name
    end
end

--[[ Flat/Table representation --]]
function LMGraph:Flat(header, outfile)
    local flatColumns = {
        "count",
        "timePercent", "time", "total_time", "timePerCount", "totalTimePerCount",
        "allocPercent", "allocated", "total_allocated", "allocPerCount", "totalAllocPerCount",
        "name", "lines",
    }

    -- For non-compressed graphs include the nodes 'depth' (i.e., distance from
    -- root), identifier, and parent identifier.
    if not header.compress_graph then
        table.insert(flatColumns, #flatColumns, "depth")
        table.insert(flatColumns, #flatColumns, "id")
        table.insert(flatColumns, #flatColumns, "parent")
    end

    local max_time = self.global.max_time
    local max_size = self.global.max_size
    local max_count = self.global.max_count
    local output_delim = (header.csv and ", ") or " "

    local timeDegree,timeSuffix,timePerDegree,timePerSuffix = self:CreateTimeUnits(header, max_time, max_count)
    local byteDegree,byteSuffix,bytePerDegree,bytePerSuffix = self:CreateByteUnits(header, max_size, max_count)

    -- Layout the functions and then sort them. Ensuring each record has each
    -- key in 'flatColumns', otherwise remove it from the flatColumns table.
    local functions = { }
    for f,funcNode in pairs(self.functions) do
        if funcNode.record ~= nil then
            functions[#functions + 1] = f
            for j=#flatColumns,1,-1 do
                local field = LMNode.Fields[flatColumns[j]]
                if field.Cat ~= "Table" and funcNode[flatColumns[j]] == nil then
                    table.remove(flatColumns, j)
                elseif field.Cat == "Table" and not header.show_lines then
                    table.remove(flatColumns, j)
                end
            end
        end
    end

    -- Sort rows by some column value.
    table.sort(functions, CreatePrioritySort(self, LMGraph.Sorting[header.sort or "count"]))

    -- Column header label to string.format() key, caching the lengths of each
    -- string to compute padding.
    local lengths,stringTable = { },{ }
    for i=1,#functions do
        local funcNode = self.functions[functions[i]]

        local row = { }
        for j=1,#flatColumns do
            local value = funcNode[flatColumns[j]]
            local field = LMNode.Fields[flatColumns[j]]
            if field.Cat == "Time" then
                row[j] = Decimal.Format(value, timeDegree)
            elseif field.Cat == "Binary" then
                row[j] = Binary.Format(value, byteDegree)
            elseif field.Cat == "TimeAverage" then
                row[j] = Decimal.Format(value, timePerDegree)
            elseif field.Cat == "BinaryAverage" then
                row[j] = Binary.Format(value, bytePerDegree)
            elseif field.Cat == "Percent" then
                row[j] = ("%1.3f"):format(value)
            elseif field.Cat == "Table" then
                local concat = table.concat(value or { }, ", ")
                row[j] = (header.csv and ("\"%s\""):format(concat)) or concat
            elseif header.csv and field.Cat == "String" then
                row[j] = ("\"%s\""):format(value)
            else
                row[j] = tostring(value)
            end
        end

        stringTable[#stringTable + 1] = row
        for j=1,#row do
            lengths[j] = math.max(lengths[j] or 1, utf8_len(row[j]) + 1)
        end
    end

    --[[ Pad the string table & columns --]]
    local columnLabels,columnUnits = { }, { }
    for i=1,#flatColumns do
        local field = LMNode.Fields[flatColumns[i]]

        local unit = ""
        if field.Cat == "Count" then
            unit = "count"
        elseif field.Cat == "Time" then
            unit = ("%s"):format(timeSuffix)
        elseif field.Cat == "Binary" then
            unit = ("%s"):format(byteSuffix)
        elseif field.Cat == "TimeAverage" then
            unit = ("%s/call"):format(timePerSuffix)
        elseif field.Cat == "BinaryAverage" then
            unit = ("%s/call"):format(bytePerSuffix)
        elseif field.Cat == "Percent" then
            unit = "%"
        end

        lengths[i] = math.max(lengths[i] or 1, utf8_len(field.Label), utf8_len(unit) + 1)
        if header.csv then
            columnUnits[i] = ("\"%s\""):format(unit)
            columnLabels[i] = ("\"%s\""):format(field.Label)
        else
            columnUnits[i] = string_pad(unit, lengths[i])
            columnLabels[i] = string_pad(field.Label, lengths[i])
        end
    end

    outfile = outfile or LMGraph.StdOut
    outfile:write_line(table.concat(columnLabels, output_delim))
    outfile:write_line(table.concat(columnUnits, output_delim))
    for i=1,#stringTable do
        local stringRecord = stringTable[i]
        for j=1,#flatColumns do
            if j ~= #flatColumns and not header.csv then
                stringRecord[j] = string_pad(stringRecord[j], lengths[j])
            end
        end
        outfile:write_line(table.concat(stringRecord, output_delim))
    end
end

--[[ Format that resembles Pepperfish --]]
function LMGraph:Pepperfish(header, outfile)
    outfile = outfile or LMGraph.StdOut
    local max_time = self.global.max_time
    local max_size = self.global.max_size
    local max_count = self.global.max_count

    -- Create SI units
    local timeDegree,timeSuffix,timePerDegree,timePerSuffix = self:CreateTimeUnits(header, max_time, max_count)
    local byteDegree,byteSuffix,bytePerDegree,bytePerSuffix = self:CreateByteUnits(header, max_size, max_count)
    local prioritySort = CreatePrioritySort(self, LMGraph.Sorting[header.sort or "count"])

    -- Layout the functions and then sort them
    local functions = { }
    for f,funcNode in pairs(self.functions) do
        if funcNode.record ~= nil then
            functions[#functions + 1] = f
        end
    end
    table.sort(functions, prioritySort)

    -- To emulate the Pepperfish layout, a shared table for format strings is
    -- used. The 'funcNode' fields are then mapped to array indexes of the
    -- shared table by a lookup table.
    local childLayout,recordLayout,recordLookup = { },{ },{ }
    local function recordAppend(key, value) recordLayout[recordLookup[key]][2] = value end
    local function recordAppendUnit(key, value) recordLayout[recordLookup[key]][3] = value end

    recordLayout[#recordLayout + 1] = { "Sample count:" } ; recordLookup.count = #recordLayout
    childLayout[#childLayout + 1] = "Child "
    childLayout[#childLayout + 1] = "" -- 2, recordName
    childLayout[#childLayout + 1] = "sampled "
    childLayout[#childLayout + 1] = "" -- 4, childRecordCount
    childLayout[#childLayout + 1] = " times. "

    if header.memory then
        recordLayout[#recordLayout + 1] = { "", "", "" } ;
        recordLayout[#recordLayout + 1] = { "Memory persists:", "", "suffix" } ; recordLookup.persists = #recordLayout
        recordLayout[#recordLayout + 1] = { "Memory allocated:", "", "suffix" } ; recordLookup.malloc = #recordLayout
        recordLayout[#recordLayout + 1] = { "Memory deallocated:", "", "suffix" } ; recordLookup.mfree = #recordLayout
        recordLayout[#recordLayout + 1] = { "Memory allocated in children:", "", "suffix" } ; recordLookup.chmalloc = #recordLayout
        recordLayout[#recordLayout + 1] = { "Memory allocated total:", "", "suffix" } ; recordLookup.chmalloctotal = #recordLayout
        recordLayout[#recordLayout + 1] = { "Total percent allocated in function:", "", "%" } ; recordLookup.fpctalloc = #recordLayout
        recordLayout[#recordLayout + 1] = { "Total percent allocated in children:", "", "%" } ; recordLookup.cpctalloc = #recordLayout
    end

    if header.instrument then
        recordLayout[#recordLayout + 1] = { "", "", "" } ;
        recordLayout[#recordLayout + 1] = { "Time spent:", "", "" } ; recordLookup.time = #recordLayout
        recordLayout[#recordLayout + 1] = { "Time spent in function:", "", "", } ; recordLookup.ftime = #recordLayout
        recordLayout[#recordLayout + 1] = { "Time spent in children:", "", "", } ; recordLookup.ctime = #recordLayout
        recordLayout[#recordLayout + 1] = { "Time spent per sample:", "", "", "/sample" } ; recordLookup.timesample = #recordLayout
        recordLayout[#recordLayout + 1] = { "Time spent in function per sample:", "", "", "/sample" } ; recordLookup.ftimesample = #recordLayout
        recordLayout[#recordLayout + 1] = { "Time spent in children per sample:", "", "", "/sample"} ; recordLookup.ctimesample = #recordLayout
        recordLayout[#recordLayout + 1] = { "Percent time spent in function:", "", "%" } ; recordLookup.pftime = #recordLayout
        recordLayout[#recordLayout + 1] = { "Percent time spent in children:", "", "%" } ; recordLookup.pctime = #recordLayout

        childLayout[#childLayout + 1] = "Took "
        childLayout[#childLayout + 1] = "" -- childRecordTime
        childLayout[#childLayout + 1] = " "
        childLayout[#childLayout + 1] = "" -- childTimeUnit
        childLayout[#childLayout + 1] = ". Averaging "
        childLayout[#childLayout + 1] = "" -- childRecordTotalTime/childRecordCount
        childLayout[#childLayout + 1] = " "
        childLayout[#childLayout + 1] = "" -- timeunit
        childLayout[#childLayout + 1] = "/exec"
    end

    for i=1,#functions do
        local funcNode = self.functions[functions[i]]
        funcNode.children[funcNode.id] = nil

        -- Setup and sort child nodes
        local sortedChildren = { }
        for cid,_ in pairs(funcNode.children) do
            sortedChildren[#sortedChildren + 1] = cid
        end
        table.sort(sortedChildren, prioritySort)

        -- Populate format table
        recordAppend("count", string.format("%4d", funcNode.count))
        if header.memory then
            recordAppendUnit("persists", byteSuffix)
            recordAppendUnit("malloc", byteSuffix)
            recordAppendUnit("mfree", byteSuffix)
            recordAppendUnit("chmalloc", byteSuffix)
            recordAppendUnit("chmalloctotal", byteSuffix)

            recordAppend("persists", Binary.Format(funcNode.allocated - funcNode.deallocated, byteDegree))
            recordAppend("malloc", Binary.Format(funcNode.allocated, byteDegree))
            recordAppend("mfree", Binary.Format(funcNode.deallocated, byteDegree))
            if #sortedChildren > 0 then
                recordAppend("chmalloc", Binary.Format(funcNode.total_allocated - funcNode.allocated, byteDegree))
                recordAppend("chmalloctotal", Binary.Format(funcNode.total_allocated, byteDegree))
                recordAppend("fpctalloc", string.format("%1.3f", funcNode.allocPercent))
                recordAppend("cpctalloc", string.format("%1.3f", funcNode.childSizePercent))
            else
                recordAppend("chmalloc", "")
                recordAppend("chmalloctotal", "")
                recordAppend("fpctalloc", string.format("%1.3f", funcNode.allocPercent))
                recordAppend("cpctalloc", "")
            end
        end

        if header.instrument then
            recordAppendUnit("time", timeSuffix)
            recordAppendUnit("ftime", timeSuffix)
            recordAppendUnit("ctime", timeSuffix)
            recordAppendUnit("timesample", timePerSuffix)
            recordAppendUnit("ftimesample", timePerSuffix)
            recordAppendUnit("ctimesample", timePerSuffix)

            if #sortedChildren > 0 then
                local delta_time = funcNode.total_time - funcNode.time
                local ctimesample = 0
                if funcNode.count ~= 0 then
                    ctimesample = (delta_time) / funcNode.count
                end

                recordAppend("time", string.format("%4.3f", Decimal.Format(funcNode.total_time, timeDegree)))
                recordAppend("ftime", string.format("%4.3f", Decimal.Format(funcNode.time, timeDegree)))
                recordAppend("ctime",  string.format("%4.3f", Decimal.Format(delta_time, timeDegree)))
                recordAppend("timesample", string.format("%4.5f", Decimal.Format(funcNode.totalTimePerCount, timePerDegree)))
                recordAppend("ftimesample", string.format("%4.5f", Decimal.Format(funcNode.timePerCount, timePerDegree)))
                recordAppend("ctimesample", string.format("%4.5f", Decimal.Format(ctimesample, timePerDegree)))
            else
                recordAppend("time", "")
                recordAppend("ftime", string.format("%4.3f", Decimal.Format(funcNode.time, timeDegree)))
                recordAppend("ctime", "")
                recordAppend("timesample", string.format("%4.3f", Decimal.Format(funcNode.totalTimePerCount, timePerDegree)))
                recordAppend("ftimesample", "")
                recordAppend("ctimesample", "")
            end

            recordAppend("pftime", string.format("%1.3f", funcNode.timePercent))
            recordAppend("pctime", string.format("%1.3f", funcNode.childTimePercent))
        end

        -- Setup column alignment
        local longestLabel,longestUnit = 0,0
        for i=1,#recordLayout do
            longestUnit = math.max(utf8_len(recordLayout[i][2]), longestUnit)
            longestLabel = math.max(utf8_len(recordLayout[i][1]), longestLabel)
        end

        -- Dump everything
        local funcName = (" %s "):format(funcNode.name)
        if funcName:len() < 42 then
            local left = math.ceil((42 - funcName:len()) / 2)
            local right = 42 - funcName:len()
            funcName = ("%s%s%s"):format(string.rep("-", left), funcName, string.rep("-", right))
        end

        outfile:write_line(("%s%s%s"):format(string.rep("-", 19), funcName, string.rep("-", 19)))
        for i=1,#recordLayout do
            local tuple = recordLayout[i]
            if tuple[2] ~= "" then
                outfile:write_line(string.format("%s %s %s",
                    string_pad(tuple[1], longestLabel + 1),
                    string_pad(tuple[2], -(longestUnit + 1)), (tuple[3] or "")
                ))
            elseif tuple[1] == "" then
                outfile:write_line("")
            end
        end

        local longestChildLabel = 0
        for i=1,#sortedChildren do
            local childNode = funcNode.children[sortedChildren[i]]
            longestChildLabel = math.max(longestChildLabel, utf8_len(childNode.name))
        end

        local added_blank = 0
        for i=1,#sortedChildren do
            local childNode = funcNode.children[sortedChildren[i]]
            if added_blank == 0 then
                added_blank = 1
                outfile:write_line("") -- extra separation line
            end

            childLayout[2] = string_pad(childNode.name, longestChildLabel + 1)
            childLayout[4] = string.format("%6d", childNode.count)
            if header.instrument then
                childLayout[7] = string.format("%4.2f", Decimal.Format(childNode.total_time, timeDegree))
                childLayout[9] = timeSuffix
                childLayout[11] = string.format("%4.5f", Decimal.Format(childNode.total_time, timePerDegree) / childNode.count)
                childLayout[13] = timeSuffix
            end

            outfile:write_line(table.concat(childLayout))
        end
        outfile:write_line("") -- extra separation line
        outfile:flush()
    end
    outfile:write_line("\nEND")
    outfile:flush()
end

--[[
    Generate a callgrind/kcachegrind compatible graph representation. Note this
    representation is generally used for when compress_graph is false.
--]]
function LMGraph:Callgrind(header, outfile)
    outfile = outfile or LMGraph.StdOut
    local sample = header.sample
    local instrument = header.instrument
    if not instrument and not sample then
        error("Callgrind format requires instrumentation")
    end

    -- Layout the functions and then sort them
    local functions = { }
    for f,funcNode in pairs(self.functions) do
        if funcNode.record ~= nil then
            functions[#functions + 1] = f
        end
    end
    table.sort(functions, CreatePrioritySort(self, LMGraph.Sorting[header.sort or "count"]))

    -- Depth-first callgrind output function.
    local CallgrindOutput = nil
    CallgrindOutput = function(node)
        if node.callgrind_reached then return end
        node.callgrind_reached = true

        local lineDefined = (node.record.linedefined < 0 and 0) or node.record.linedefined
        outfile:write_line(("fn=(%d)"):format(node.callgrind_id))
        if instrument then
            outfile:write_line(("%d %d"):format(lineDefined, node.time))
        else
            outfile:write_line(("%d %d"):format(lineDefined, node.count))
        end

        if header.compress_graph then -- Ensure cycles don't exist
            node.children[node.id] = nil
        end

        for cid,childNode in pairs(node.children) do
            local count = (childNode.count <= 0 and 1) or childNode.count
            outfile:write_line(("cfn=(%d)"):format(childNode.callgrind_id))
            outfile:write_line(("calls=%d %d"):format(count, 0))
            if instrument then
                outfile:write_line(("%d %d"):format(childNode.record.parent_line, childNode.total_time))
            else
                outfile:write_line(("%d %d"):format(childNode.record.parent_line, childNode.child_count))
            end
        end

        outfile:write_line("")
        for cid,childNode in pairs(node.children) do
            CallgrindOutput(childNode)
        end
    end

    outfile:write_line(("events: %s"):format((instrument and "Time") or "Samples"))
    outfile:write_line("")
    outfile:write_line("# define function ID mapping")
    for i=1,#functions do
        local funcNode = self.functions[functions[i]]
        local funcName = (funcNode.record.func == "0" and "main") or funcNode.name
        local funcSource = funcNode.name:match("(%b())")
        if funcSource ~= nil then
            funcNode.callgrind_source = funcSource:sub(2, funcSource:len() - 1)
        end

        -- Generate a 'pseudo' name to ensure KCachegrind doesn't confuse or
        -- unnecessarily compress nodes. Functions are first-class values so
        -- emulate multiple export symbols.
        if not header.compress_graph then
            funcName = ("%s:%s"):format(funcName, funcNode.id)
        end

        funcNode.callgrind_id = i
        outfile:write_line(("fn=(%d) %s"):format(funcNode.callgrind_id, funcName))
        outfile:write_line(("fl=(%d) %s"):format(funcNode.callgrind_id, funcNode.callgrind_source or "[C]"))
    end

    outfile:write_line("")
    outfile:write_line("# define callgraph")
    CallgrindOutput(self.root)
end

Citizen.Graph = LMGraph -- cfxLua places LMGraph into "Citizen" instead of returning.