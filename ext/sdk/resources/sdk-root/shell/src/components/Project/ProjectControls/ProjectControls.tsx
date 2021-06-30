import React from 'react';
import classnames from 'classnames';
import { ServerButton } from './ServerButton/ServerButton';
import { BuildButton } from './BuildButton';
import { NewResource } from './NewResource';
import { NewDirectory } from './NewDirectory';
import { ImportAsset } from './ImportAsset';
import { projectSettingsIcon } from 'constants/icons';
import { ProjectState } from 'store/ProjectState';
import { observer } from 'mobx-react-lite';
import { AiOutlinePlus } from 'react-icons/ai';
import { NewMap } from './NewMap';
import { StatusState } from 'store/StatusState';
import { Feature } from 'shared/api.types';
import s from './ProjectControls.module.scss';

export const ProjectControls = observer(function ProjectControls() {
  const worldEditorAvailable = StatusState.getFeature(Feature.worldEditor);

  return (
    <>
      <button
        className={s.item}
        onClick={ProjectState.openSettings}
        data-label="Project settings"
      >
        {projectSettingsIcon}
      </button>

      <div className={s['item-stack']}>
        <button className={s.item}>
          <AiOutlinePlus style={{ fontSize: '1.1rem' }} />
        </button>

        <div className={s.stack}>
          <NewDirectory className={s.item} />
          <NewResource className={s.item} />
          {worldEditorAvailable && (
            <NewMap className={s.item} />
          )}
        </div>
      </div>

      <ImportAsset className={s.item} />
      <BuildButton className={s.item} />
      <ServerButton className={s.item} />
    </>
  );
});
