import React from 'react';
import classnames from 'classnames';
import sBaseTool from '../BaseTool/BaseTool.module.scss';
import { Title } from 'components/controls/Title/Title';
import { BsPlay } from 'react-icons/bs';
import { WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import { WEHotkeysState } from 'personalities/WorldEditorPersonality/store/WEHotkeysState';
import { WECommand } from 'personalities/WorldEditorPersonality/constants/commands';
import { observer } from 'mobx-react-lite';

export const PlayButton = observer(function PlayButton() {
  const rootClassName = classnames(sBaseTool.toggle, sBaseTool.hoverable);

  const shortcut = WEHotkeysState.getCommandHotkey(WECommand.ACTION_ENTER_PLAYTEST_MODE);

  return (
    <Title animated={false} delay={0} fixedOn="top" title="Enter play mode" shortcut={shortcut}>
      {(ref) => (
        <button
          ref={ref}
          className={rootClassName}
          onClick={WEState.enterPlaytestMode}
        >
          <BsPlay />
        </button>
      )}
    </Title>
  );
});
