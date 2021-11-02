import React from 'react';
import classnames from 'classnames';
import { ResourceTemplateDescriptor } from 'resource-templates/types';
import s from './ResourceTemplate.module.scss';

export interface ResourceTemplateProps {
  descriptor: ResourceTemplateDescriptor,

  checked?: boolean,
  onClick?: () => void,
}

export const ResourceTemplate = React.memo(function ResourceTemplate(props: ResourceTemplateProps) {
  const isEnabled = props.descriptor.useIsEnabled
    ? props.descriptor.useIsEnabled()
    : true;

  const rootClassName = classnames(s.root, {
    [s.checked]: props.checked,
    [s.disabled]: !isEnabled,
  });

  const iconNode = typeof props.descriptor.icon === 'string'
    ? <img src={props.descriptor.icon} alt={props.descriptor.title} />
    : props.descriptor.icon;

  const description = !isEnabled && props.descriptor.disabledDescription
    ? props.descriptor.disabledDescription
    : props.descriptor.description;

  return (
    <div className={rootClassName} onClick={props.onClick}>
      <div className={s.icon}>
        {iconNode}
      </div>
      <div className={s.content}>
        <div className={s.title}>
          {props.descriptor.title}
        </div>
        {!!description && (
          <div className={s.description}>
            {description}
          </div>
        )}
      </div>
    </div>
  );
});
