local function isRetailBuild()
    return os.getenv('CFX_RETAIL_BUILD') == 'true'
end

local function isFeatureBranchBuild()
    return os.getenv('CFX_FEATURE_BRANCH_BUILD') == 'true'
end

local function isRetailOrFeatureBranchBuild()
    -- return isRetailBuild() or isFeatureBranchBuild()
    return false
end

local function ensureTrailingSlash(str)
    if str:sub(-1) ~= '/' then
        return str .. '/'
    end

    return str
end

local function ensureNoTrailingSlash(str)
    if str:sub(-1) == '/' then
        return str:sub(1, -2)
    end

    return str
end

local function envVarDefine(options)
    local envVarName = options.env

    if not envVarName then
        envVarName = options.define
    end

    local envVarValue = os.getenv(envVarName)

    if envVarValue then
        if options.process then
            envVarValue = options.process(envVarValue)
        end

        printf("Setting %s to %s", options.define, envVarValue)

        defines {
            options.define .. '="' .. envVarValue .. '"'
        }
    elseif options.required then
        error(envVarName .. " not set, but is required.")
    end
end

return function()
    envVarDefine {
        env = "CFX_CNL_ENDPOINT",
        define = "CNL_ENDPOINT",
        process = ensureTrailingSlash
    }
    envVarDefine {
        env = "CFX_CNL_HB_ENDPOINT",
        define = "CNL_HB_ENDPOINT",
        process = ensureTrailingSlash
    }

    if _OPTIONS['game'] == 'server' then
        envVarDefine {
            define = "CFX_SENTRY_SERVER_CRASH_UPLOAD_URL",
            required = isRetailOrFeatureBranchBuild()
        }
        envVarDefine {
            define = "CFX_SENTRY_SERVER_HANGS_UPLOAD_URL",
            required = isRetailOrFeatureBranchBuild()
        }
    else
        envVarDefine {
            env = iif(_OPTIONS['game'] == 'rdr3', "CFX_SENTRY_SESSION_URL_REDM", "CFX_SENTRY_SESSION_URL_FIVEM"),
            define = "CFX_SENTRY_SESSION_URL",
            required = isRetailOrFeatureBranchBuild()
        }
        envVarDefine {
            env = iif(_OPTIONS['game'] == 'rdr3', "CFX_SENTRY_SESSION_KEY_REDM", "CFX_SENTRY_SESSION_KEY_FIVEM"),
            define = "CFX_SENTRY_SESSION_KEY",
            required = isRetailOrFeatureBranchBuild()
        }
        envVarDefine {
            define = "CFX_CRASH_INGRESS_URL",
            process = ensureNoTrailingSlash
        }
        envVarDefine {
            env = "CFX_POLICY_LIVE_ENDPOINT",
            define = "POLICY_LIVE_ENDPOINT",
            process = ensureTrailingSlash
        }
        envVarDefine {
            define = "CFX_UPDATER_URL",
            process = ensureNoTrailingSlash
        }
    end
end
