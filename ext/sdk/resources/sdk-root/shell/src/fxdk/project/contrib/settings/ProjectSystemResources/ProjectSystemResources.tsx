import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { SystemResourceDescriptor, systemResourcesDescriptors } from './system-resources';
import { SystemResource, SYSTEM_RESOURCES_NAMES } from 'backend/system-resources/system-resources-constants';
import { Project } from 'fxdk/project/browser/state/project';
import s from './ProjectSystemResources.module.scss';

export const ProjectSystemResources = observer(function ProjectSystemResources() {
  const nodes = Object.entries(systemResourcesDescriptors).map(([categoryName, descriptors]) => {
    const resourcesNodes = Object.entries(descriptors).map(([resourceName, descriptor]: [SystemResource, SystemResourceDescriptor]) => {
      const handleClick = () => Project.toggleSystemResource(resourceName);

      return (
        <div
          key={resourceName}
          onClick={handleClick}
          className={classnames(s.resource, { [s.active]: Project.manifest.systemResources.includes(resourceName) })}
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
      <div key={categoryName} className={s.category}>
        <div className="modal-label">
          {categoryName}:
        </div>
        <div className="modal-block">
          <div className={s.list}>
            {resourcesNodes}
          </div>
        </div>
      </div>
    );
  });



  return (
    <div className={s.wrapper}>
      {nodes}
    </div>
  );
});
