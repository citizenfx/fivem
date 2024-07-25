import { ValueOrGetter } from '@cfx-dev/ui-components';
import React from 'react';

type ValueGetSet<T> = {
  accessors(): {
    getValue(): T;
    setValue(value: T): void;
  };
};

interface IBaseSetting {
  /**
   * Can be raw label string or l10n key
   */
  label: React.ReactNode;

  /**
   * Can be raw description string or l10n key
   */
  description?: React.ReactNode;

  /**
   * Wether or not this setting is visible/enabled
   */
  visible?(): boolean;
}

export namespace ISetting {
  export type Text = IBaseSetting &
    ValueGetSet<string> & {
      type: 'text';

      /**
       * Default is text
       */
      inputType?: 'text' | 'number';

      placeholder?: ValueOrGetter<string>;
    };

  export type Checkbox = IBaseSetting &
    ValueGetSet<boolean> & {
      type: 'checkbox';
    };

  export type Button = IBaseSetting & {
    type: 'button';

    onClick(): void;
  };

  export type Select = IBaseSetting &
    ValueGetSet<string> & {
      type: 'select';

      options: ValueOrGetter<Record<string, ValueOrGetter<string>>>;
    };
  export type Switch = IBaseSetting &
    ValueGetSet<string> & {
      type: 'switch';

      multiline?: boolean;

      options: ValueOrGetter<Record<string, ValueOrGetter<string>>>;
    };

  export type DisplayNode = IBaseSetting & {
    type: 'displayNode';

    node: ValueOrGetter<React.ReactNode>;
  };

  export type AnySetting =
    | Text
    | Checkbox
    | Button
    | Select
    | Switch
    | DisplayNode
    | (IBaseSetting & { render: () => React.ReactNode });
}

export type ICategory = {
  icon: React.ReactNode;
  label: React.ReactNode;
  settings: Map<string, ISetting.AnySetting>;
};

export type ISettings = Map<string, ICategory>;
