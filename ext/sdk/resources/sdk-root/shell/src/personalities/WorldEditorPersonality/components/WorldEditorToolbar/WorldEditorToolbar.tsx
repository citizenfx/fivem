import React from 'react';
import { observer } from "mobx-react-lite";
import classnames from 'classnames';
import { WEState } from '../../store/WEState';
import { StatusTool } from './StatusTool/StatusTool';
import { ModeSelector } from './ModeSelector/ModeSelector';
import { GameState } from 'store/GameState';
import { PatchesTool } from './PatchesTool/PatchesTool';
import { AdditionsTool } from './AdditionsTool/AdditionsTool';
import { AddObjectTool } from './AddObjectTool/AddObjectTool';
import s from './WorldEditorToolbar.module.scss';
import sBaseTool from './BaseTool/BaseTool.module.scss';
import { closeIcon } from 'constants/icons';
import { ActiveSelectionTool } from './ActiveSelectionTool/ActiveSelectionTool';
import { EnvironmentTool } from './EnvironmentTool/EnvironmentTool';
import { FlashingMessage } from './FlashingMessage/FlashingMessage';
// import { DebugRestartSdkGameTool } from './DebugRestartSdkGameTool';
import { PropertiesTool } from './PropertiesTool/PropertiesTool';
import { SettingsTool } from './SettingsTool/SettingsTool';
import { Title } from 'components/controls/Title/Title';

function CloseButton() {
  const rootClassName = classnames(sBaseTool.toggle, sBaseTool.labelled, sBaseTool.hoverable);

  return (
    <Title animated={false} delay={0} fixedOn="top" title="Close World Editor">
      {(ref) => (
        <button
          ref={ref}
          className={rootClassName}
          onClick={WEState.closeMap}
        >
          {closeIcon}
        </button>
      )}
    </Title>
  );
}

export const WorldEditorToolbar = observer(function WorldEditorToolbar() {
  const showControls = WEState.ready && !GameState.archetypesCollectionPending;

  return (
    <>
      <div className={s['top-left']}>
        {!!WEState.map && showControls && (
          <>
            <PatchesTool />
            <AdditionsTool />

            <div /> {/* gap */}

            <AddObjectTool />
          </>
        )}

        {/* <DebugRestartSdkGameTool /> */}
      </div>

      <div className={s['top-center']}>
        <ActiveSelectionTool />
      </div>

      <FlashingMessage />

      <div className={s['top-right']}>
        {showControls && (
          <SettingsTool />
        )}

        {showControls && (
          <EnvironmentTool />
        )}

        <div /> {/* gap */}

        <StatusTool />

        <CloseButton />
      </div>

      <div className={s['left-bottom']}>
        {!!WEState.map && showControls && (
          <PropertiesTool />
        )}
      </div>

      <div className={s.bottom}>
        {showControls && (
          <ModeSelector />
        )}
      </div>
    </>
  );
});
