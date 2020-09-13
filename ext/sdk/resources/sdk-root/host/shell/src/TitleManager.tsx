import React from 'react';
import { ProjectContext } from './contexts/ProjectContext';

const titleBase = 'CitizenFX Development Kit (FiveM)';

export const TitleManager = React.memo(() => {
  const { project } = React.useContext(ProjectContext);
  const titleRef = React.useRef<HTMLTitleElement | null>();

  React.useEffect(() => {
    if (titleRef.current) {
      const title = project
        ? `${project.manifest.name} — ${titleBase}`
        : titleBase;

      titleRef.current.innerText = title;
    }
  }, [project]);

  React.useLayoutEffect(() => {
    titleRef.current = document.querySelector('title');
  }, [titleRef]);

  return null;
});
