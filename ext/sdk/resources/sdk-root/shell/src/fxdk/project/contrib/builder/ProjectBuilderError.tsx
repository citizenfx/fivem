import * as React from 'react';
import { Output } from 'fxdk/ui/Output/Output';
import { ProjectBuildError } from 'shared/project.types';
import { OutputState } from 'store/OutputState';
import s from './ProjectBuilder.module.scss';
import { observer } from 'mobx-react-lite';

export interface ProjectBuilderErrorProps {
  error: ProjectBuildError,
}

export const ProjectBuilderError = observer(function ProjectBuilderError(props: ProjectBuilderErrorProps) {
  const { error } = props;

  if (error.type === 'generic') {
    return (
      <>
        {error.data}
      </>
    );
  }

  if (error.type === 'assetBuildError') {
    return (
      <div className={s['asset-build-error']}>
        <div>
          Build command of {error.data.assetName} asset has failed.
          <br/>
          Output of <kbd>{OutputState.getLabel(error.data.outputChannelId) || error.data.outputChannelId}</kbd>:
        </div>
        <Output
          channelId={error.data.outputChannelId}
        />
      </div>
    );
  }

  return null;
});
