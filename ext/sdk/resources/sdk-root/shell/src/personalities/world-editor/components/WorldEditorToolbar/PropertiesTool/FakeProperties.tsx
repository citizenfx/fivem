import React from 'react';
import { BsCameraVideo, BsChevronBarDown } from 'react-icons/bs';
import { additionsToolIcon, rotateIcon, scaleIcon, translateIcon } from 'personalities/world-editor/constants/icons';
import { NumberInput } from 'fxdk/ui/controls/NumberInput/NumberInput';
import { div } from 'utils/styled';
import { deleteIcon } from 'fxdk/ui/icons';
import { IntroForceRecalculate } from 'fxdk/ui/Intro/Intro';
import { PropertiesToolSection } from './PropertiesTool.Section';
import s from './PropertiesTool.module.scss';

const Header = div(s, 'header');
const Icon = div(s, 'icon');
const Name = div(s, 'name');
const Controls = div(s, 'controls');
const Block = div(s, 'block');
const LabelRow = div(s, 'label-row');
const Label = div(s, 'label');
const Control = div<{ inputs?: boolean }>(s, 'control');

const noop = () => { };

export const FakeProperties = function FakeProperties() {
  return (
    <>
      <IntroForceRecalculate />

      <Header>
        <Icon>
          {additionsToolIcon}
        </Icon>
        <Name>
          <span>
            prop_fake_taxi_l
          </span>
        </Name>
        <Controls>
          <button>
            {deleteIcon}
          </button>
          <button>
            <BsCameraVideo />
          </button>
        </Controls>
      </Header>

      <PropertiesToolSection
        open
        title="Transform & position"
      >
        <Block>
          <LabelRow>
            <Label>
              {translateIcon} Position:
            </Label>
            <Controls>
              <button>
                <BsChevronBarDown />
              </button>
            </Controls>
          </LabelRow>
          <Control inputs>
            <NumberInput
              label="x:"
              value={31.9235}
              onChange={noop}
              className={s.input}
            />
            <NumberInput
              label="y:"
              value={45.0023}
              onChange={noop}
              className={s.input}
            />
            <NumberInput
              label="z:"
              value={1.4273}
              onChange={noop}
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
              value={90}
              onChange={noop}
              className={s.input}
            />
            <NumberInput
              label="y:"
              value={0.0989}
              onChange={noop}
              className={s.input}
            />
            <NumberInput
              label="z:"
              value={0}
              onChange={noop}
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
              <button>
                1:1
              </button>
            </Controls>
          </LabelRow>
          <Control inputs>
            <NumberInput
              label="x:"
              value={1}
              onChange={noop}
              className={s.input}
            />
            <NumberInput
              label="y:"
              value={1}
              onChange={noop}
              className={s.input}
            />
            <NumberInput
              label="z:"
              value={1}
              onChange={noop}
              className={s.input}
            />
          </Control>
        </Block>
      </PropertiesToolSection>
    </>
  );
};
