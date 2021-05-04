import * as React from 'react';
import { ServerButton } from 'components/ServerButton/ServerButton';
import { BuildButton } from './BuildButton';
import { NewResource } from './NewResource';
import { NewDirectory } from './NewDirectory';
import { ImportAsset } from './ImportAsset';
import s from './ProjectControls.module.scss';

export const ProjectControls = React.memo(function ProjectControls() {
  return (
    <>
      <NewDirectory className={s.item} />
      <NewResource className={s.item} />
      <ImportAsset className={s.item} />
      <BuildButton className={s.item} />
      <ServerButton />
    </>
  );
});
