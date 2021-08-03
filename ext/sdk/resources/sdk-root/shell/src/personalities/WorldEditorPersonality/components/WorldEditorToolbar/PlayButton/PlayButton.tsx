import React from 'react';
import classnames from 'classnames';
import sBaseTool from '../BaseTool/BaseTool.module.scss';
import { Title } from 'components/controls/Title/Title';
import { BsPlay } from 'react-icons/bs';
import { WEState } from 'personalities/WorldEditorPersonality/store/WEState';

export function PlayButton() {
  const rootClassName = classnames(sBaseTool.toggle, sBaseTool.hoverable);

  return (
    <Title animated={false} delay={0} fixedOn="top" title="Enter play mode" shortcut="F5">
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
}
