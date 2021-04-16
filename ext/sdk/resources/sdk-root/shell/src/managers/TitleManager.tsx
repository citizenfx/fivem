import React from 'react';
import { observer } from 'mobx-react-lite';
import { ProjectState } from 'store/ProjectState';

const titleBase = 'Cfx.re Development Kit (FiveM)';

export const TitleManager = observer(() => {
  const titleRef = React.useRef<HTMLTitleElement | null>();

  React.useEffect(() => {
    if (titleRef.current) {
      const title = ProjectState.hasProject
        ? `${ProjectState.projectName} — ${titleBase}`
        : titleBase;

      titleRef.current.innerText = title;
    }
  }, [ProjectState.hasProject, ProjectState.projectName]);

  React.useLayoutEffect(() => {
    titleRef.current = document.querySelector('title');
  }, [titleRef]);

  return null;
});
