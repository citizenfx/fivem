-- This resource is part of the default Cfx.re asset pack (cfx-server-data)
-- Altering or recreating for local use only is strongly discouraged.

version '1.0.0'
author 'Cfx.re <root@cfx.re>'
description 'Builds resources with webpack. To learn more: https://webpack.js.org'
repository 'https://github.com/citizenfx/cfx-server-data'

dependency 'yarn'
server_script 'webpack_builder.js'

fx_version 'adamant'
game 'common'
