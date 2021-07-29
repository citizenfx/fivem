import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { EditorMode, WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import { rotateIcon, scaleIcon, translateIcon } from 'personalities/WorldEditorPersonality/constants/icons';
import s from './ModeSelector.module.scss';
import { Title } from 'components/controls/Title/Title';

export const ModeSelector = observer(function ModeSelector() {
  return (
    <div className={s.root}>
      <Title
        delay={0}
        animated={false}
        fixedOn="top"
        title={`Switch to ${WEState.editorLocal ? 'global' : 'local'} coords`}
        shortcut="~"
      >
        {(ref) => (
          <div ref={ref} className={classnames(s.toggle, s.hoverable)} onClick={WEState.toggleEditorLocal}>
            <div className={classnames(s.coords, { [s.global]: !WEState.editorLocal })}>
              <div className={s.axis} />
              <div className={s.object} />
            </div>
          </div>
        )}
      </Title>

      <div className={s.modes}>
        <Title
          delay={0}
          animated={false}
          fixedOn="top"
          title="Translate"
          shortcut="1"
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
          shortcut="2"
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
          shortcut="3"
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
