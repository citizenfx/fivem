import React from 'react';
import { ServerButton } from './ServerButton/ServerButton';
import { BuildButton } from './BuildButton';
import { NewResource } from './NewResource';
import { NewDirectory } from './NewDirectory';
import { ImportAsset } from './ImportAsset';
import { projectSettingsIcon } from 'constants/icons';
import { ProjectState } from 'store/ProjectState';
import { observer } from 'mobx-react-lite';
import s from './ProjectControls.module.scss';

export const ProjectControls = observer(function ProjectControls() {
  return (
    <>
      <button
        className={s.item}
        onClick={ProjectState.openSettings}
        data-label="Project settings"
      >
        {projectSettingsIcon}
      </button>
      <NewDirectory className={s.item} />
      <NewResource className={s.item} />
      <ImportAsset className={s.item} />
      <BuildButton className={s.item} />
      <ServerButton className={s.item} />
    </>
  );
});
