import React from 'react';
import { ProjectContext } from '../../contexts/ProjectContext';
import { Button } from '../controls/Button/Button';

import s from './Welcome.module.scss';


export const Welcome = React.memo(() => {
  const { openCreator, openOpener } = React.useContext(ProjectContext);

  return (
    <div className={s.root}>
      <div className={s.header}>
        <h1>
          Welcome to FxDK
        </h1>
        <span>
          The present and the future of development for cfx.re platform
        </span>
      </div>

      <div className={s.controls}>
        <Button
          text="Create New Project"
          theme="primary"
          onClick={openCreator}
        />
        <Button
          text="Open Project"
          onClick={openOpener}
        />
      </div>
    </div>
  );
});
