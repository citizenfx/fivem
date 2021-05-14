import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { SystemResourceDescriptor, systemResourcesDescriptors } from './system-resources';
import { ProjectState } from 'store/ProjectState';
import { SystemResource, SYSTEM_RESOURCES_NAMES } from 'backend/system-resources/system-resources-constants';
import s from './ProjectSystemResources.module.scss';

export const ProjectSystemResources = observer(function ProjectSystemResources() {
  const nodes = Object.entries(systemResourcesDescriptors).map(([categoryName, descriptors]) => {
    const resourcesNodes = Object.entries(descriptors).map(([resourceName, descriptor]: [SystemResource, SystemResourceDescriptor]) => {
      const handleClick = () => ProjectState.project.toggleSystemResource(resourceName);

      return (
        <div
          key={resourceName}
          onClick={handleClick}
          className={classnames(s.resource, { [s.active]: ProjectState.project.manifest.systemResources.includes(resourceName) })}
        >
          <div className={s.row}>
            <div className={s.name}>
              {SYSTEM_RESOURCES_NAMES[resourceName]}
            </div>
          </div>

          <div className={s.description}>
            {descriptor.description}
          </div>
        </div>
      );
    });

    return (
      <React.Fragment key={categoryName}>
        <div className="modal-label">
          {categoryName}:
        </div>
        <div className="modal-block">
          <div className={s.root}>
            {resourcesNodes}
          </div>
        </div>
      </React.Fragment>
    );
  });



  return <>{nodes}</>;
});
