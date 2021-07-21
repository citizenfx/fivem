import React from 'react';
import { IoReloadSharp } from 'react-icons/io5';
import { serverApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import s from './BaseTool/BaseTool.module.scss';

export function DebugRestartSdkGameTool() {
  return (
    <button className={s.toggle} onClick={() => sendApiMessage(serverApi.restartResource, 'sdk-game')}>
      <IoReloadSharp />
    </button>
  );
}
