import * as React from 'react';
import { ServerButton } from 'components/ServerButton/ServerButton';
import { BuildButton } from './BuildButton';
import { NewResource } from './NewResource';
import s from './ProjectToolbar.module.scss';
import { NewDirectory } from './NewDirectory';

export const ProjectToolbar = React.memo(function ProjectToolbar() {
  return (
    <>
      <NewDirectory className={s.item} />
      <NewResource className={s.item} />
      <BuildButton className={s.item} />
      <ServerButton />
    </>
  );
});
