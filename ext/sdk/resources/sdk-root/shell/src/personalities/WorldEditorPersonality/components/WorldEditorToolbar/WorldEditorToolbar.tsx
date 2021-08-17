import React from 'react';
import { observer } from "mobx-react-lite";
import classnames from 'classnames';
import { WEMode, WEState } from '../../store/WEState';
import { StatusTool } from './StatusTool/StatusTool';
import { ModeSelector } from './ModeSelector/ModeSelector';
import { GameState } from 'store/GameState';
import { PatchesTool } from './PatchesTool/PatchesTool';
import { AdditionsTool } from './AdditionsTool/AdditionsTool';
import { AddObjectTool } from './AddObjectTool/AddObjectTool';
import { closeIcon } from 'constants/icons';
import { ActiveSelectionTool } from './ActiveSelectionTool/ActiveSelectionTool';
import { EnvironmentTool } from './EnvironmentTool/EnvironmentTool';
import { FlashingMessage } from './FlashingMessage/FlashingMessage';
import { PropertiesTool } from './PropertiesTool/PropertiesTool';
import { SettingsTool } from './SettingsTool/SettingsTool';
import { Title } from 'components/controls/Title/Title';
import { PlayButton } from './PlayButton/PlayButton';
import { PlayExitMessage } from './PlayExitMessage/PlayExitMessage';
import s from './WorldEditorToolbar.module.scss';
import sBaseTool from './BaseTool/BaseTool.module.scss';
import { DebugRestartSdkGameTool } from './DebugRestartSdkGameTool';

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

  const rootClassName = classnames(s.root, {
    [s.hidden]: WEState.mode === WEMode.PLAYTEST,
  });

  return (
    <div className={rootClassName}>
      <PlayExitMessage />

      <div className={classnames(s['top-left'], s.hideable)} data-intro-id="top-left-toolbar">
        {!!WEState.map && showControls && (
          <>
            <PatchesTool />
            <AdditionsTool />

            <div /> {/* gap */}

            <AddObjectTool />
          </>
        )}

        <DebugRestartSdkGameTool />
      </div>

      <div className={classnames(s['top-center'], s.hideable)}>
        <ActiveSelectionTool />
      </div>

      <FlashingMessage />

      <div className={classnames(s['top-right'], s.hideable)}>
        {showControls && (
          <>
            <PlayButton />

            <div /> {/* gap */}

            <EnvironmentTool />
            <SettingsTool />
          </>
        )}

        <div /> {/* gap */}

        <StatusTool />

        <CloseButton />
      </div>

      <div className={classnames(s['left-bottom'], s.hideable)}>
        {!!WEState.map && showControls && (
          <PropertiesTool />
        )}
      </div>

      <div className={classnames(s['bottom'], s.hideable)}>
        {showControls && (
          <ModeSelector />
        )}
      </div>
    </div>
  );
});
