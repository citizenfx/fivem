import React from 'react';
import classnames from 'classnames';
import { observer } from "mobx-react-lite";
import { Checkbox } from 'fxdk/ui/controls/Checkbox/Checkbox';
import { Input } from 'fxdk/ui/controls/Input/Input';
import { Project } from 'fxdk/project/browser/state/project';
import { computeVariables, Conflict } from './variablesCompiler';
import { ConvarKind, IConvarCategory, IConvarEntry, ProjectVariableSetter } from 'fxdk/project/common/project.types';
import { useOpenFlag } from 'utils/hooks';
import { Select } from 'fxdk/ui/controls/Select/Select';
import { RangeInput } from 'fxdk/ui/controls/RangeInput/RangeInput';
import s from './ProjectVariables.module.scss';
import { BsExclamationCircle, BsExclamationTriangle } from 'react-icons/bs';
import { Title } from 'fxdk/ui/controls/Title/Title';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { deleteIcon } from 'fxdk/ui/icons';
import { noop } from 'fxdk/base/functional';

const errorHint = (
  <>
    Conflicting definition
    <br />
    <br />
    To fix either make all definitions compatible
    <br />
    or change name, <strong>namespacing works best</strong>
  </>
);

function getSetterLabel(setter: ProjectVariableSetter): string {
  switch (setter) {
    case ProjectVariableSetter.INFORMATION: return 'Information';
    case ProjectVariableSetter.REPLICATED: return 'Replicated';
    case ProjectVariableSetter.SERVER_ONLY: return 'Server only';
  }
}

export const ProjectResourceSettings = observer(function ProjectResourceSettings() {
  const { categoriesMap, typeConflicts, strandedVariables } = computeVariables();

  const categories = Object.entries(categoriesMap).map(([name, category]) => {
    return (
      <Category key={name} name={name} category={category} conflicts={typeConflicts} />
    );
  });

  return (
    <div className={s.root}>
      {Boolean(Object.keys(strandedVariables).length) && Object.entries(strandedVariables).map(([variable, { setter, value }]) => (
        <div className={s.stranded}>
          <div className={s.header}>
            <div className={s.title}>
              <div className={s.icon}>
                <BsExclamationTriangle />
              </div>

              <div className={s.tuple}>
                {variable}
                <span className={s.setter}>
                  {getSetterLabel(setter)}
                </span>
              </div>
            </div>

            <div className={s.subtitle}>
              This variable is not defined by any asset, most likely a leftover
            </div>
          </div>

          <div className={s.value}>
            <Input
              disabled
              size="small"
              value={String(value)}
              onChange={noop}
            />
          </div>

          <div className={s.control}>
            <Button
              size="small"
              icon={deleteIcon}
              onClick={() => Project.deleteVariable(variable)}
            />
          </div>
        </div>
      ))}

      {Object.entries(typeConflicts).map(([name, conflict]) => (
        <RConflict key={name} name={name} conflict={conflict} />
      ))}

      {categories}
    </div>
  );
});

const RConflict = (function RConflict(props: { name: string, conflict: Conflict }) {
  const { name, conflict } = props;

  return (
    <div className={s.conflict}>
      <div className={s.header}>
        <div className={s.title}>
          <Title fixedOn="right" delay={0} title={errorHint}>
            {(ref) => (
              <div ref={ref} className={s.icon}>
                <BsExclamationCircle />
              </div>
            )}
          </Title>
          {name}
        </div>
        <div className={s.subtitle}>
          Incompatible definitions:
        </div>
      </div>

      <ul>
        {Object.values(conflict).flat().map(({ categoryName, entry }) => (
          <li key={`${categoryName}-${entry.setter}-${entry.kind}`}>
            <div className={s.setter}>
              {getSetterLabel(entry.setter)}
            </div>
            <div className={s.type}>
              {entry.kind}
            </div>
            <div>
              under&nbsp;
              <span
                className={s.link}
                onClick={() => document.getElementById(categoryName + entry.variable)?.scrollIntoView(true)}
              >
                {categoryName}
              </span>
            </div>
          </li>
        ))}
      </ul>
    </div>
  );
});

const Category = observer(function Category(props: { name: string, category: IConvarCategory, conflicts: Record<string, Conflict> }) {
  const { name, category, conflicts } = props;

  const [expanded, _expand, _collapse, toggleExpanded] = useOpenFlag(true);

  const categoryClassName = classnames(s.category, {
    [s.expanded]: expanded,
  });

  const variableNodes = category.entries.map((entry) => (
    <Variable
      key={entry.title}
      entry={entry}
      conflict={conflicts[entry.variable]}
      categoryName={name}
    />
  ));

  return (
    <div className={categoryClassName} id={name}>
      <div className={s.header}>
        <div className={s.title}>
          {name}
        </div>

        <div className={s.subtitle}>
          {category.subtitle}
        </div>
      </div>

      {expanded && (
        <div className={s.variables}>
          {variableNodes}
        </div>
      )}
    </div>
  );
});

const Variable = observer(function Variable(props: { categoryName: string, entry: IConvarEntry, conflict: Conflict | undefined }) {
  const { entry, categoryName, conflict } = props;

  return (
    <div className={classnames(s.entry, { [s.error]: !!conflict })} id={categoryName + entry.variable}>
      <div className={s.header}>
        <div className={s.title}>
          {!!conflict && (
            <Title fixedOn="right" delay={0} title={errorHint}>
              {(ref) => (
                <div ref={ref} className={s.icon}>
                  <BsExclamationCircle />
                </div>
              )}
            </Title>
          )}

          <div className={s.tuple}>
            {entry.variable}
            <span className={s.setter}>{getSetterLabel(entry.setter)}</span>
          </div>
        </div>
        <div className={s.subtitle}>
          {entry.title}
        </div>
      </div>
      <div className={s.control}>
        {getVariableControl(entry, !!conflict)}
      </div>
      <div className={s.control}>
        <Button
          size="small"
          icon={deleteIcon}
          disabled={typeof Project.manifest.variables?.[entry.variable] === 'undefined'}
          onClick={() => Project.deleteVariable(entry.variable)}
        />
      </div>
    </div>
  );
});

function getVariableControl(entry: IConvarEntry, disabled: boolean): React.ReactNode {
  const handleChange = React.useCallback((value) => {
    Project.setVariable(entry.variable, entry.setter, value);
  }, [entry]);

  switch (entry.kind) {
    case ConvarKind.Bool: {
      return (
        <Checkbox
          value={Boolean(valueOrDefault(entry))}
          onChange={handleChange}
          disabled={disabled}
        />
      );
    }

    case ConvarKind.Int: return (
      <Input
        size="small"
        type="number"
        value={String(valueOrDefault(entry))}
        onChange={(value) => handleChange(parseInt(value, 10))}
        pattern={/^[0-9]+$/}
        min={entry.minValue}
        max={entry.maxValue}
        disabled={disabled}
      />
    );

    case ConvarKind.Combi:
    case ConvarKind.Slider: return (
      <>
        <RangeInput
          size="small"
          value={Number(valueOrDefault(entry))}
          onChange={handleChange}
          min={entry.minValue}
          max={entry.maxValue}
          disabled={disabled}
        />
        <Input
          size="small"
          type="number"
          value={String(valueOrDefault(entry))}
          onChange={(value) => handleChange(parseInt(value, 10))}
          pattern={/^[0-9]+$/}
          min={entry.minValue}
          max={entry.maxValue}
          disabled={disabled || (entry.kind !== ConvarKind.Combi)}
        />
      </>
    );

    case ConvarKind.String: return (
      <Input
        size="small"
        value={String(valueOrDefault(entry))}
        onChange={handleChange}
        disabled={disabled}
      />
    );

    case ConvarKind.Multi: return (
      <Select
        size="small"
        value={String(valueOrDefault(entry))}
        options={entry.entries.map((v) => ({ value: v, title: v }))}
        onChange={handleChange}
        disabled={disabled}
      />
    );
  }
}

function valueOrDefault(entry: IConvarEntry): string | number | boolean {
  const value = Project.manifest.variables?.[entry.variable]?.value;

  if (typeof value !== 'undefined') {
    return value;
  }

  if (entry.kind === ConvarKind.Multi) {
    return entry.entries[0];
  }

  return entry.defaultValue;
}
