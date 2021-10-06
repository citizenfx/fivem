import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { EditorMode, WEState } from 'personalities/world-editor/store/WEState';
import { rotateIcon, scaleIcon, translateIcon } from 'personalities/world-editor/constants/icons';
import { Title } from 'fxdk/ui/controls/Title/Title';
import { WEHotkeysState } from 'personalities/world-editor/store/WEHotkeysState';
import { WECommand } from 'personalities/world-editor/constants/commands';
import s from './ModeSelector.module.scss';

export const ModeSelector = observer(function ModeSelector() {
  const coordSystemShortcut = WEHotkeysState.getCommandHotkey(WECommand.CONTROL_COORD_SYSTEM_TOGGLE);
  const translateShortcut = WEHotkeysState.getCommandHotkey(WECommand.CONTROL_MODE_TRANSLATE_TOGGLE);
  const rotateShortcut = WEHotkeysState.getCommandHotkey(WECommand.CONTROL_MODE_ROTATE_TOGGLE);
  const scaleShortcut = WEHotkeysState.getCommandHotkey(WECommand.CONTROL_MODE_SCALE_TOGGLE);

  return (
    <div className={s.root}>
      <Title
        delay={0}
        animated={false}
        fixedOn="top"
        title={`Switch to ${WEState.editorLocal ? 'global' : 'local'} coords`}
        shortcut={coordSystemShortcut}
      >
        {(ref) => (
          <div
            ref={ref}
            className={classnames(s.toggle, s.hoverable)}
            onClick={WEState.toggleEditorLocal}
            data-intro-id="coord-system"
          >
            <div className={classnames(s.coords, { [s.global]: !WEState.editorLocal })}>
              <div className={s.axis} />
              <div className={s.object} />
            </div>
          </div>
        )}
      </Title>

      <div className={s.modes} data-intro-id="mode-selector">
        <Title
          delay={0}
          animated={false}
          fixedOn="top"
          title="Translate"
          shortcut={translateShortcut}
        >
          {(ref) => (
            <div
              ref={ref}
              onClick={WEState.enableTranslation}
              className={classnames(s.mode, { [s.active]: WEState.editorMode === EditorMode.TRANSLATE })}
            >
              {translateIcon}
            </div>
          )}
        </Title>

        <Title
          delay={0}
          animated={false}
          fixedOn="top"
          title="Rotate"
          shortcut={rotateShortcut}
        >
          {(ref) => (
            <div
              ref={ref}
              onClick={WEState.enableRotation}
              className={classnames(s.mode, { [s.active]: WEState.editorMode === EditorMode.ROTATE })}
            >
              {rotateIcon}
            </div>
          )}
        </Title>

        <Title
          delay={0}
          animated={false}
          fixedOn="top"
          title="Scale"
          shortcut={scaleShortcut}
        >
          {(ref) => (
            <div
              ref={ref}
              onClick={WEState.enableScaling}
              className={classnames(s.mode, { [s.active]: WEState.editorMode === EditorMode.SCALE })}
            >
              {scaleIcon}
            </div>
          )}
        </Title>
      </div>
    </div >
  );
});
