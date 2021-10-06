import React from 'react';
import { observer } from 'mobx-react-lite';
import { ProjectState } from 'store/ProjectState';
import { WEState } from 'personalities/world-editor/store/WEState';

const titleBase = 'Cfx.re Development Kit (FiveM)';

export const TitleManager = observer(() => {
  const titleRef = React.useRef<HTMLTitleElement | null>();

  React.useEffect(() => {
    if (titleRef.current) {
      const parts = [titleBase];

      if (ProjectState.hasProject) {
        parts.push(ProjectState.projectName);
      }

      if (WEState.mapName) {
        parts.push(WEState.mapName);
      }

      titleRef.current.innerText = parts.reverse().join(' — ');
    }
  }, [ProjectState.hasProject, ProjectState.projectName, WEState.mapName]);

  React.useLayoutEffect(() => {
    titleRef.current = document.querySelector('title');
  }, [titleRef]);

  return null;
});
