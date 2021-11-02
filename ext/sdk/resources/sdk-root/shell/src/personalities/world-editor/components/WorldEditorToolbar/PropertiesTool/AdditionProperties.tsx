import React from 'react';
import { WEEntityMatrixIndex } from 'backend/world-editor/world-editor-types';
import { observer } from 'mobx-react-lite';
import { WEState } from 'personalities/world-editor/store/WEState';
import { applyScale, eulerFromMatrix, getScale } from 'shared/math';
import { BsCameraVideo, BsChevronBarDown } from 'react-icons/bs';
import { additionsToolIcon, rotateIcon, scaleIcon, translateIcon } from 'personalities/world-editor/constants/icons';
import { NumberInput } from 'fxdk/ui/controls/NumberInput/NumberInput';
import { div } from 'utils/styled';
import { deleteIcon, duplicateIcon, infoIcon } from 'fxdk/ui/icons';
import { Title } from 'fxdk/ui/controls/Title/Title';
import { WEHotkeysState } from 'personalities/world-editor/store/WEHotkeysState';
import { WECommand } from 'personalities/world-editor/constants/commands';
import s from './PropertiesTool.module.scss';
import { makeAutoObservable } from 'mobx';
import { LocalStorageValue } from 'store/generic/LocalStorageValue';
import { PropertiesToolSection } from './PropertiesTool.Section';
import { SmallInput } from 'fxdk/ui/controls/SmallInput/SmallInput';

const Header = div(s, 'header');
const Icon = div(s, 'icon');
const Name = div(s, 'name');
const Controls = div(s, 'controls');
const Block = div(s, 'block');
const LabelRow = div(s, 'label-row');
const Label = div(s, 'label');
const Control = div<{ inputs?: boolean }>(s, 'control');

const CREATED_EVENT_INFO = (
  <div>
    Event with this name will be triggered
    <br/>
    when this addition gets <strong>created</strong>
    <br/>
    <br/>
    Addition handle will be in first argument
    <br/>
    <br/>
    Leave it empty to disable
  </div>
);
const DELETED_EVENT_INFO = (
  <div>
    Event with this name will be triggered
    <br/>
    when this addition gets <strong>deleted</strong>
    <br/>
    <br/>
    Leave it empty to disable
  </div>
);

namespace Buttons {
  export const Duplicate = ({ additionId }) => (
    <Title delay={0} animated={false} fixedOn="top" title="Duplicate" shortcut={WEHotkeysState.getCommandHotkey(WECommand.ACTION_DUPLICATE_SELECTION)}>
      {(ref) => (
        <button ref={ref} onClick={() => WEState.map!.duplicateAddition(additionId)}>
          {duplicateIcon}
        </button>
      )}
    </Title>
  );

  export const Delete = ({ additionId }) => (
    <Title delay={0} animated={false} fixedOn="top" title="Delete" shortcut={WEHotkeysState.getCommandHotkey(WECommand.ACTION_DELETE_SELECTION)}>
      {(ref) => (
        <button ref={ref} onClick={() => WEState.map!.deleteAddition(additionId)}>
          {deleteIcon}
        </button>
      )}
    </Title>
  );

  export const FocusInView = () => (
    <Title delay={0} animated={false} fixedOn="top" title="Focus in view" shortcut={WEHotkeysState.getCommandHotkey(WECommand.ACTION_FOCUS_SELECTION_IN_VIEW)}>
      {(ref) => (
        <button ref={ref} onClick={WEState.focusSelectionInView}>
          <BsCameraVideo />
        </button>
      )}
    </Title>
  );

  export const SetOnGround = ({ additionId }) => (
    <Title
      delay={0}
      animated={false}
      fixedOn="right"
      title="Set on ground"
      shortcut={WEHotkeysState.getCommandHotkey(WECommand.ACTION_SET_ADDITION_ON_GROUND)}
    >
      {(ref) => (
        <button
          ref={ref}
          onClick={() => WEState.map!.setAdditionOnGround(additionId)}
        >
          <BsChevronBarDown />
        </button>
      )}
    </Title>
  );

  export const ScaleToOriginal = ({ additionId }) => (
    <Title delay={0} animated={false} fixedOn="right" title="Scale to original size">
      {(ref) => (
        <button
          ref={ref}
          onClick={() => WEState.map!.setAdditionScale(additionId, 1, 1, 1)}
        >
          1:1
        </button>
      )}
    </Title>
  );
}

export interface AdditionProprtiesProps {
  additionId: string,
}

export const AdditionProprties = observer(function AdditionProprties(props: AdditionProprtiesProps) {
  const { additionId } = props;
  const addition = WEState.map!.additions[additionId];

  const [sx, sy, sz] = getScale(addition.mat);

  const unscaledMat = Array.from(addition.mat);
  applyScale(unscaledMat, [1, 1, 1]);

  const px = addition.mat[WEEntityMatrixIndex.AX];
  const py = addition.mat[WEEntityMatrixIndex.AY];
  const pz = addition.mat[WEEntityMatrixIndex.AZ];

  const [rz, rx, ry] = eulerFromMatrix(unscaledMat);

  return (
    <>
      <Header>
        <Icon>
          {additionsToolIcon}
        </Icon>
        <Name>
          <span>
            {addition.label}
          </span>
        </Name>
        <Controls>
          <Buttons.Duplicate additionId={additionId} />
          <Buttons.Delete additionId={additionId} />
          <Buttons.FocusInView />
        </Controls>
      </Header>

      <PropertiesToolSection
        title="Transform & position"
        open={AdditionPropertiesState.transformSectionOpen}
        onOpenToggle={AdditionPropertiesState.toggleTransformSectionOpen}
      >
        <Block>
          <LabelRow>
            <Label>
              {translateIcon} Position:
            </Label>
            <Controls>
              <Buttons.SetOnGround additionId={additionId} />
            </Controls>
          </LabelRow>
          <Control inputs>
            <NumberInput
              label="x:"
              value={px}
              onChange={(px) => WEState.map!.setAdditionPosition(additionId, px, py, pz)}
              className={s.input}
            />
            <NumberInput
              label="y:"
              value={py}
              onChange={(py) => WEState.map!.setAdditionPosition(additionId, px, py, pz)}
              className={s.input}
            />
            <NumberInput
              label="z:"
              value={pz}
              onChange={(pz) => WEState.map!.setAdditionPosition(additionId, px, py, pz)}
              className={s.input}
            />
          </Control>
        </Block>

        <Block>
          <Label>
            {rotateIcon}
            Rotation:
          </Label>
          <Control inputs>
            <NumberInput
              label="x:"
              value={rx}
              onChange={(rx) => WEState.map!.setAdditionRotation(additionId, rx, ry, rz)}
              className={s.input}
            />
            <NumberInput
              label="y:"
              value={ry}
              onChange={(ry) => WEState.map!.setAdditionRotation(additionId, rx, ry, rz)}
              className={s.input}
            />
            <NumberInput
              label="z:"
              value={rz}
              onChange={(rz) => WEState.map!.setAdditionRotation(additionId, rx, ry, rz)}
              className={s.input}
            />
          </Control>
        </Block>

        <Block>
          <LabelRow>
            <Label>
              {scaleIcon}
              Scale:
            </Label>
            <Controls>
              <Buttons.ScaleToOriginal additionId={additionId} />
            </Controls>
          </LabelRow>
          <Control inputs>
            <NumberInput
              label="x:"
              value={sx}
              onChange={(sx) => WEState.map!.setAdditionScale(additionId, sx, sy, sz)}
              className={s.input}
            />
            <NumberInput
              label="y:"
              value={sy}
              onChange={(sy) => WEState.map!.setAdditionScale(additionId, sx, sy, sz)}
              className={s.input}
            />
            <NumberInput
              label="z:"
              value={sz}
              onChange={(sz) => WEState.map!.setAdditionScale(additionId, sx, sy, sz)}
              className={s.input}
            />
          </Control>
        </Block>
      </PropertiesToolSection>

      <PropertiesToolSection
        title="Events"
        open={AdditionPropertiesState.eventsSectionOpen}
        onOpenToggle={AdditionPropertiesState.toggleEventsSectionOpen}
      >
        <Block>
          <LabelRow>
            <Label>
              Created:
            </Label>
            <Controls>
              <Title delay={0} fixedOn="right" title={CREATED_EVENT_INFO}>
                {(ref) => (
                  <span ref={ref}>
                    {infoIcon}
                  </span>
                )}
              </Title>
            </Controls>
          </LabelRow>
          <Control>
            <SmallInput
              value={addition.evcreated || ''}
              onChange={(evcreated) => WEState.map!.setAdditionCreatedEvent(additionId, evcreated)}
            />
          </Control>
        </Block>
        <Block>
        <LabelRow>
            <Label>
              Deleted:
            </Label>
            <Controls>
              <Title delay={0} fixedOn="right" title={DELETED_EVENT_INFO}>
                {(ref) => (
                  <span ref={ref}>
                    {infoIcon}
                  </span>
                )}
              </Title>
            </Controls>
          </LabelRow>
          <Control>
            <SmallInput
              value={addition.evdeleted || ''}
              onChange={(evdeleted) => WEState.map!.setAdditionDeletedEvent(additionId, evdeleted)}
            />
          </Control>
        </Block>
      </PropertiesToolSection>
    </>
  );
});

const AdditionPropertiesState = new class AdditionPropertiesState {
  private transformSectionOpenValue = new LocalStorageValue({
    key: 'we:addition-properties:transform-open',
    defaultValue: true,
  });

  private eventsSectionOpenValue = new LocalStorageValue({
    key: 'we:addition-properties:events-open',
    defaultValue: true,
  });

  get transformSectionOpen(): boolean {
    return this.transformSectionOpenValue.get();
  }
  set transformSectionOpen(open: boolean) {
    this.transformSectionOpenValue.set(open);
  }

  get eventsSectionOpen(): boolean {
    return this.eventsSectionOpenValue.get();
  }
  set eventsSectionOpen(open: boolean) {
    this.eventsSectionOpenValue.set(open);
  }

  constructor() {
    makeAutoObservable(this);
  }

  readonly toggleTransformSectionOpen = () => {
    this.transformSectionOpen = !this.transformSectionOpen;
  };

  readonly toggleEventsSectionOpen = () => {
    this.eventsSectionOpen = !this.eventsSectionOpen;
  };
}();
