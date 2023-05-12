import { ISetting } from "cfx/common/services/settings/types";
import { Button } from "cfx/ui/Button/Button";
import { Checkbox } from "cfx/ui/Checkbox/Checkbox";
import { Input } from "cfx/ui/Input/Input";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Select } from "cfx/ui/Select/Select";
import { Switch } from "cfx/ui/Switch/Switch";
import { Text } from "cfx/ui/Text/Text";
import { getValue } from "cfx/utils/getValue";
import { observer } from "mobx-react-lite";

interface ControlProps {
  setting: ISetting.AnySetting,
}
export const SettingItem = observer(function SettingItem(props: ControlProps) {
  const {
    setting
  } = props;

  return (
    <Flex vertical>
      <Text size="large" opacity="75">
        {getValue(setting.label)}
      </Text>

      <div>
        {getControl(setting)}
      </div>
    </Flex>
  );
});

function getControl(setting: ISetting.AnySetting): React.ReactNode {
  if (!('type' in setting)) {
    return setting.render();
  }

  // @DEPRECATED
  switch (setting.type) {
    case 'text': {
      const accessors = setting.accessors();

      return (
        <Input
          type={setting.inputType || 'text'}
          value={accessors.getValue()}
          onChange={accessors.setValue}
        />
      );
    }

    case 'button': {
      return (
        <Button
          text={getValue(setting.description || '')}
          onClick={setting.onClick}
        />
      );
    }

    case 'displayNode': {
      return getValue(setting.node);
    }

    case 'select': {
      const accessors = setting.accessors();

      return (
        <Select
          value={accessors.getValue()}
          onChange={accessors.setValue}
          options={Object.entries(getValue(setting.options)).map(([value, label]) => ({
            value,
            label: getValue(label),
          }))}
        />
      );
    }

    case 'switch': {
      const accessors = setting.accessors();

      return (
        <Switch
          multiline={setting.multiline}
          value={accessors.getValue()}
          onChange={accessors.setValue}
          options={Object.entries(getValue(setting.options)).map(([value, label]) => ({
            value,
            label: getValue(label),
          }))}
        />
      );
    }

    case 'checkbox': {
      const accessors = setting.accessors();

      return (
        <Checkbox
          value={accessors.getValue()}
          onChange={accessors.setValue}
          label={getValue(setting.description)}
        />
      );
    }

    default: {
      //                                               because when all types are covered in switch
      //                                               TS thinks `setting` is `never`, which is correct,
      //                                               but what if we add more types to the definition?
      throw new Error(`Unknown setting type ${(setting as any).type}`);
    }
  }
}
