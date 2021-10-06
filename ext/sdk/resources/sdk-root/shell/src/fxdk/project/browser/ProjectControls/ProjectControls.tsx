import React from 'react';
import classnames from 'classnames';
import { ServerButton } from './ServerButton/ServerButton';
import { NewDirectory } from './NewDirectory';
import { projectSettingsIcon } from 'constants/icons';
import { observer } from 'mobx-react-lite';
import { AiOutlinePlus } from 'react-icons/ai';
import { Title } from 'fxdk/ui/controls/Title/Title';
import { useIntroIsFocusedNode } from 'fxdk/ui/Intro/Intro';
import { ProjectParticipants } from '../projectExtensions';
import { ShellCommands } from 'shell-api/commands';
import s from './ProjectControls.module.scss';
import { ProjectSettingsCommands } from 'fxdk/project/contrib/settings/settings.commands';

const CREATE_PROJECT_ITEM_INTRO_NODE_ID = 'create-project-item';

export const ProjectControls = observer(function ProjectControls() {
  const createProjectItemFocusedInIntro = useIntroIsFocusedNode(CREATE_PROJECT_ITEM_INTRO_NODE_ID);
  const createProjectItemClassName = classnames(s['item-stack'], {
    [s.focused]: createProjectItemFocusedInIntro,
  });

  const itemCreatorNodes = ProjectParticipants.getAllEnabledItemCreators().map((creator) => {
    const handleClick = () => ShellCommands.invoke(creator.commandId);

    return (
      <Title key={creator.id} animated={false} delay={0} title={creator.label} fixedOn="right">
        {(ref) => (
          <button
            ref={ref}
            className={s.item}
            onClick={handleClick}
          >
            {creator.icon}
          </button>
        )}
      </Title>
    );
  });

  const controlNodes = ProjectParticipants.getAllEnabledControls().map((control) => {
    const handleClick = () => ShellCommands.invoke(control.commandId);

    return (
      <Title key={control.id} animated={false} delay={0} title={control.label} fixedOn="bottom">
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
          <NewDirectory className={s.item} />

          {itemCreatorNodes}
        </div>
      </div>

      {controlNodes}

      <ServerButton className={s.item} />
    </>
  );
});
