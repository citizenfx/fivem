import React from 'react';
import { Api } from 'fxdk/browser/Api';
import { IoReloadSharp } from 'react-icons/io5';
import { serverApi } from 'shared/api.events';
import s from './BaseTool/BaseTool.module.scss';

export function DebugRestartSdkGameTool() {
  return (
    <button className={s.toggle} onClick={() => Api.send(serverApi.restartResource, 'sdk-game')}>
      <IoReloadSharp />
    </button>
  );
}
