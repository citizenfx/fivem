import React from 'react';
import classnames from 'classnames';
import { ServerButton } from './ServerButton/ServerButton';
import { projectSettingsIcon } from 'fxdk/ui/icons';
import { observer } from 'mobx-react-lite';
import { AiOutlinePlus } from 'react-icons/ai';
import { Title, TitleProps } from 'fxdk/ui/controls/Title/Title';
import { useIntroIsFocusedNode } from 'fxdk/ui/Intro/Intro';
import { IProjectControlParticipant, IProjectStackedControlsParticipant, ProjectParticipants } from '../projectExtensions';
import { ShellCommands } from 'shell-api/commands';
import { ProjectSettingsCommands } from 'fxdk/project/contrib/settings/settings.commands';
import { ProjectExplorerParticipants } from '../../contrib/explorer/projectExplorerExtensions';
import { ExplorerRuntime } from '../../contrib/explorer/explorer.runtime';
import s from './ProjectControls.module.scss';

const CREATE_PROJECT_ITEM_INTRO_NODE_ID = 'create-project-item';

export const ProjectControls = observer(function ProjectControls() {
  const createProjectItemFocusedInIntro = useIntroIsFocusedNode(CREATE_PROJECT_ITEM_INTRO_NODE_ID);
  const createProjectItemClassName = classnames(s['item-stack'], {
    [s.focused]: createProjectItemFocusedInIntro,
  });

  const itemCreatorNodes = ProjectExplorerParticipants.getAllItemCreators().map((creator) => {
    const commandId = ExplorerRuntime.getOrCreateRootItemCreatorCommandId(creator);
    const icon = typeof creator.icon === 'function'
      ? creator.icon('')
      : creator.icon;

    const handleClick = () => ShellCommands.invoke(commandId);

    return (
      <Title key={creator.id} animated={false} delay={0} title={`New ${creator.label}`} fixedOn="right">
        {(ref) => (
          <button
            ref={ref}
            className={s.item}
            onClick={handleClick}
          >
            {icon}
          </button>
        )}
      </Title>
    );
  });

  const controlNodes = ProjectParticipants.getAllEnabledControls().map((control) => {
    if ('controls' in control) {
      return renderStackedControl(control);
    }

    return renderNormalControl(control);
  });

  return (
    <>
      <Title animated={false} delay={0} fixedOn="bottom" title="Project settings">
        {(ref) => (
          <button
            ref={ref}
            className={s.item}
            onClick={() => ShellCommands.invoke(ProjectSettingsCommands.OPEN)}
          >
            {projectSettingsIcon}
          </button>
        )}
      </Title>

      <div className={createProjectItemClassName}>
        <button className={s.item}>
          <AiOutlinePlus style={{ fontSize: '1.1rem' }} />
        </button>

        <div className={s.stack} data-intro-id={CREATE_PROJECT_ITEM_INTRO_NODE_ID}>
          {itemCreatorNodes}
        </div>
      </div>

      {controlNodes}

      <ServerButton className={s.item} />
    </>
  );
});

function renderStackedControl(control: IProjectStackedControlsParticipant): React.ReactNode {
  return (
    <div key={control.id} className={s['item-stack']}>
      <button className={s.item}>
        {control.icon}
      </button>

      <div className={s.stack} data-intro-id={control.introId}>
        {control.controls.map((control) => renderNormalControl(control, 'right'))}
      </div>
    </div>
  );
}

function renderNormalControl(control: IProjectControlParticipant, titleFixedOn: TitleProps['fixedOn'] = 'bottom'): React.ReactNode {
  if (control.enabled && !control.enabled()) {
    return null;
  }

  const handleClick = () => ShellCommands.invokeWithArgs(control.commandId, control.commandArgs);

  return (
    <Title key={control.id} animated={false} delay={0} title={control.label} fixedOn={titleFixedOn}>
      {(ref) => (
        <button
          ref={ref}
          className={s.item}
          onClick={handleClick}
          data-intro-id={control.introId || ''}
        >
          {control.icon}
        </button>
      )}
    </Title>
  );
}
