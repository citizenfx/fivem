import * as React from 'react';
import { Output } from 'components/Output/Output';
import { ProjectBuildError } from 'shared/project.types';
import s from './ProjectBuilder.module.scss';
import { OutputContext } from 'contexts/OutputContext';

export interface ProjectBuilderErrorProps {
  error: ProjectBuildError,
}

export const ProjectBuilderError = React.memo(function ProjectBuilderError(props: ProjectBuilderErrorProps) {
  const { error } = props;

  const { outputsLabels } = React.useContext(OutputContext);

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
          Output of <kbd>{outputsLabels[error.data.outputChannelId] || error.data.outputChannelId}</kbd>:
        </div>
        <Output
          channelId={error.data.outputChannelId}
        />
      </div>
    );
  }

  return null;
});
