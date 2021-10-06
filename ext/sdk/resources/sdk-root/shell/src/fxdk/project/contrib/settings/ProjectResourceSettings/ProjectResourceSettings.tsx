import React from 'react';
import { observer } from "mobx-react-lite";
import { ProjectState } from 'store/ProjectState';
import { Checkbox } from 'fxdk/ui/controls/Checkbox/Checkbox';
import { projectApi } from 'shared/api.events';
import { Input } from 'fxdk/ui/controls/Input/Input';
import { Switch } from 'fxdk/ui/controls/Switch/Switch';
import { Api } from 'fxdk/browser/Api';

type ConVarType = 'CV_BOOL' | 'CV_INT' | 'CV_SLIDER' | 'CV_COMBI' | 'CV_STRING' | 'CV_PASSWORD' | 'CV_MULTI';

const Setting = observer(function Setting({ setting }: { setting: any }) {
  const project = ProjectState.project;
  const [title, id, type, def, ...more]: [string, string, ConVarType, any, ...any] = setting;

  const variable = project.manifest.variables?.[id] ?? def;
  const updateVariable = React.useCallback((value: string) => {
    Api.send(projectApi.setVariable, { key: id, value: value + '' });
  }, [id]);

  if (type === 'CV_BOOL') {
    const setVariable = (value: boolean) => {
      updateVariable(value ? 'true' : 'false');
    };

    return <div className="modal-block"><Checkbox
      value={variable === '1' || variable === 'true' || variable === true}
      onChange={setVariable}
      label={title}
    /></div>
  } else if (type === 'CV_STRING') {
    return <div className="modal-block"><Input
      value={variable}
      onChange={updateVariable}
      label={title}
    /></div>
  } else if (type === 'CV_INT') {
    return <div className="modal-block"><Input
      value={variable}
      onChange={updateVariable}
      label={title}
      pattern={/^[0-9]+$/}
    /></div>
  } else if (type === 'CV_PASSWORD') {
    return <div className="modal-block"><Input
      value={variable}
      onChange={updateVariable}
      label={title}
      type="password"
    /></div>
  } else if (type === 'CV_MULTI') {
    return <>
      <div className="modal-label">{title}</div>
      <div className="modal-block">
        <Switch
          value={variable}
          options={def.map(option => ({
            label: option[0],
            value: option[1]
          }))}
          onChange={updateVariable}
        />
      </div>
    </>
  } else if (type === 'CV_SLIDER' || type === 'CV_COMBI') {
    return <div className="modal-block"><Input
      type="range"
      value={variable}
      combi={type === 'CV_COMBI'}
      min={more[0]}
      max={more[1]}
      onChange={updateVariable}
      label={title}
    /></div>
  }

  return <span>Unsupported type: {type}</span>;
});

export const ProjectResourceSettings = observer(function ProjectResourceSettings() {
  const project = ProjectState.project;
  const assetDefs = project.assetDefs;
  const convarCategories = Object.entries(assetDefs)
    .map(([_, value]) => value?.convarCategories);

  const categoryMapping = {};
  for (const categories of convarCategories) {
    if (!categories) {
      continue;
    }

    for (const [title, data] of categories) {
      const [subtitle, entries] = data;
      if (!categoryMapping[title]) {
        categoryMapping[title] = {
          subtitle,
          entries
        };
      } else {
        categoryMapping[title].entries.push(...entries);
      }
    }
  }

  const entries = (data: any[]) => {
    return (<>
      {data.map(setting => <Setting setting={setting} />)}
    </>);
  };

  return <>
    {Object.entries(categoryMapping)
      .sort((a, b) => a[0].localeCompare(b[0]))
      .map(([title, data]: [string, any]) => <>
        <div className="modal-category">
          <h1>{title}</h1>
          {data.subtitle && <h2>{data.subtitle}</h2>}
        </div>
        {entries(data.entries)}
      </>)}
  </>;
});
