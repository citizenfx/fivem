import { SystemResource } from 'backend/system-resources/system-resources-constants';
import React from 'react';

export interface SystemResourceDescriptor {
  name: string,
  description: React.ReactNode,
}

export type Category = string;

export const systemResourcesDescriptors: Record<Category, Partial<Record<SystemResource, SystemResourceDescriptor>>> = {
  'Maps': {
    [SystemResource.MAP_HIPSTER]: {
      name: 'Map hipster',
      description: 'Example spawn points for FiveM with a "hipster" model',
    },
    [SystemResource.MAP_SKATER]: {
      name: 'Map skater',
      description: 'Example spawn points for FiveM with a "skater" model',
    },
  },

  'Builders': {
    [SystemResource.WEBPACK]: {
      name: 'Webpack builder',
      description: (
        <>
          Builds resources with webpack. To learn more: <a href="https://webpack.js.org">https://webpack.js.org</a>
        </>
      ),
    },
    [SystemResource.YARN]: {
      name: 'Yarn builder',
      description: (
        <>
          Builds resources with yarn. To learn more: <a href="https://classic.yarnpkg.com">https://classic.yarnpkg.com</a>
        </>
      ),
    },
  },

  'Chat': {
    [SystemResource.CHAT]: {
      name: 'Chat',
      description: 'Provides baseline chat functionality using a NUI-based interface',
    },
    [SystemResource.CHAT_THEME_GTAO]: {
      name: 'GTA:O chat theme',
      description: 'A GTA Online-styled theme for the chat resource',
    },
  },

  'System managers': {
    [SystemResource.MAP_MANAGER]: {
      name: 'Map manager',
      description: 'A flexible handler for game type/map association',
    },
    [SystemResource.SPAWN_MANAGER]: {
      name: 'Spawn manager',
      description: 'Handles spawning a player in a unified fashion to prevent resources from having to implement custom spawn logic',
    },
    [SystemResource.SESSION_MANAGER]: {
      name: 'Session manager',
      description: 'Handles the "host lock" for non-OneSync servers',
    },
  },

  'Basics': {
    [SystemResource.BASIC_GAMEMODE]: {
      name: 'Basic gamemode',
      description: 'A basic freeroam gametype that uses the default spawn logic from spawnmanager',
    },
    [SystemResource.BASE_EVENTS]: {
      name: 'Base events',
      description: 'Adds basic events for developers to use in their scripts. Some third party resources may depend on this resource',
    },
    [SystemResource.PLAYER_DATA]: {
      name: 'Player data',
      description: 'A basic resource for storing player identifiers',
    },
    [SystemResource.PLAYER_NAMES]: {
      name: 'Player names',
      description: 'A basic resource for displaying player names',
    },
  },

  'Others': {
    [SystemResource.RCON_LOG]: {
      name: 'RCON log',
      description: 'Handles old-style server player management commands',
    },
    [SystemResource.RUNCODE]: {
      name: 'Runcode',
      description: (
        <>
          Allows server owners to execute arbitrary server-side or client-side JavaScript/Lua code.
          <br/>
          <strong>Consider only using this on development servers</strong>
        </>
      ),
    },
    [SystemResource.HARD_CAP]: {
      name: 'Hard cap',
      description: 'Limits the number of players to the amount set by sv_maxclients in your server.cfg',
    },
  },
};
