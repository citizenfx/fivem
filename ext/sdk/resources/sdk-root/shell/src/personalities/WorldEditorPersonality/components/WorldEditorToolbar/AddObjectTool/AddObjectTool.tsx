import React from 'react';
import { observer } from 'mobx-react-lite';
import { BaseTool } from '../BaseTool/BaseTool';
import { WETool } from '../WEToolbarState';
import { Indicator } from 'components/Indicator/Indicator';
import { ObjectsBrowser } from './ObjectsBrowser';
import { FiPlus } from 'react-icons/fi';
import { ArchetypesState } from 'personalities/WorldEditorPersonality/store/ArchetypesState';
import { addObjectToolIcon } from 'personalities/WorldEditorPersonality/constants/icons';

export const AddObjectTool = observer(function AddObjectTool() {
  const [archetypesLoaded, setArchetypesLoaded] = React.useState(ArchetypesState.archetypes.length > 0);

  // Query all archetypes
  React.useEffect(() => {
    let stillMounted = true;

    const rAF = requestAnimationFrame(() => {
      ArchetypesState.load().then(() => {
        if (stillMounted) {
          setArchetypesLoaded(true);
        }
      });
    });

    return () => {
      stillMounted = false;
      cancelAnimationFrame(rAF);
    }
  }, []);

  return (
    <BaseTool
      tool={WETool.AddObject}
      icon={archetypesLoaded ? addObjectToolIcon : <Indicator />}
      label="Add object"
    >
      <ObjectsBrowser />
    </BaseTool>
  );
});
