import * as React from 'react';
import { ProjectContext } from '../../../../contexts/ProjectContext';
import { ProjectItemProps } from '../ProjectExplorer.item';
import s from './File.module.scss';
import { getFileIcon } from './File.utils';

export const File = React.memo((props: ProjectItemProps) => {
  const { entry } = props;

  const { openFile } = React.useContext(ProjectContext);

  const handleClick = React.useCallback(() => {
    openFile(entry);
  }, [entry, openFile]);

  const icon = getFileIcon(entry);

  return (
    <div className={s.root} onClick={handleClick}>
      {icon}
      {entry.name}
    </div>
  );
});
