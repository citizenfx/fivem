-- global loader bits
local _i, _f, _v, _r, _ri, _rf, _rl, _s, _rv, _ro, _in, _ii, _fi =
	Citizen.PointerValueInt(), Citizen.PointerValueFloat(), Citizen.PointerValueVector(),
	Citizen.ReturnResultAnyway(), Citizen.ResultAsInteger(), Citizen.ResultAsFloat(), Citizen.ResultAsLong(), Citizen.ResultAsString(), Citizen.ResultAsVector(), Citizen.ResultAsObject(),
	Citizen.InvokeNative, Citizen.PointerValueIntInitialized, Citizen.PointerValueFloatInitialized

local g = _G
local rg = rawget
local rs = rawset
local _ln = Citizen.LoadNative
local load = load
local msgpack = msgpack
local _tostring = tostring
local type = type
local function _ts(num)
	if num == 0 or not num then -- workaround for users calling string parameters with '0', also nil being translated
		return nil
	end
	return _tostring(num)
end
local function _ch(hash)
	if g.type(hash) == 'string' then
		return g.GetHashKey(hash)
	end

	return hash
end

local function _mfr(fn)
	return g.Citizen.GetFunctionReference(fn)
end

local Global = setmetatable({}, { __newindex = function(_, n, v)
	g[n] = v

	rs(_, n, v)
end})

local nativeEnv = {
    Global = Global,
    _mfr = _mfr,
    _ch = _ch,
    _ts = _ts,
    msgpack = msgpack,
    rs = rs,
    _i = _i,
    _f = _f,
    _v = _v,
    _r = _r,
    _ri = _ri,
    _rf = _rf,
    _rl = _rl,
    _s = _s,
    _rv = _rv,
    _ro = _ro,
    _in = _in,
    _ii = _ii,
    _fi = _fi
}

local nilCache = {}

setmetatable(g, {
    __index = function(t, n)
        local v = rg(t, n)

        if not v and not nilCache[n] then
            local nativeString = _ln(n)

            if nativeString then
				if type(nativeString) == 'function' then
					rs(t, n, nativeString)

					v = nativeString
				else
					local chunk = load(nativeString, '@' .. n .. '.lua', 't', nativeEnv)

					if chunk then
						chunk()

						v = rg(t, n)
					end
				end
            else
                nilCache[n] = true
            end
        end

        return v
    end
})