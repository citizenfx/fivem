import React from 'react';
import { observer } from 'mobx-react-lite';
import { ProjectState } from 'store/ProjectState';
import { WorldEditorState } from 'personalities/WorldEditorPersonality/WorldEditorState';

const titleBase = 'Cfx.re Development Kit (FiveM)';

export const TitleManager = observer(() => {
  const titleRef = React.useRef<HTMLTitleElement | null>();

  React.useEffect(() => {
    if (titleRef.current) {
      const parts = [titleBase];

      if (ProjectState.hasProject) {
        parts.push(ProjectState.projectName);
      }

      if (WorldEditorState.mapName) {
        parts.push(WorldEditorState.mapName);
      }

      titleRef.current.innerText = parts.reverse().join(' — ');
    }
  }, [ProjectState.hasProject, ProjectState.projectName, WorldEditorState.mapName]);

  React.useLayoutEffect(() => {
    titleRef.current = document.querySelector('title');
  }, [titleRef]);

  return null;
});
