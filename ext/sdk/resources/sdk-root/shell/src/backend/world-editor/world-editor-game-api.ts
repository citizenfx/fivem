import { worldEditorApi } from "shared/api.events";
import {
  WEAckEnvironmentRequest,
  WEApplyAdditionChangeRequest,
  WEApplyPatchChangeRequest,
  WECam,
  WECreateAdditionRequest,
  WECreatePatchRequest,
  WEDeletePatchRequest,
  WEMap,
  WESelection,
  WESetAdditionRequest,
  WESetEnvironmentRequest,
  WESettingsChangeRequest,
} from "./world-editor-types";

export const WEApi = Object.freeze({
  Ready: define<void>('we:ready'),
  Map: define<WEMap>('we:map'),
  Accept: define<void>('we:accept'),

  SetCam: define<WECam>(worldEditorApi.setCam),
  SetCamBaseMultiplier: define<number>('we:setCamBaseMultiplier'),

  FocusInView: define<{ cam: WECam, lookAt: [number, number, number] }>('we:focusInView'),
  ObjectPreview: define<string>('we:objectPreview'),

  Selection: define<WESelection>('we:selection'),

  EnterPlaytestMode: define<void>('we:enterPlaytestMode'),
  ExitPlaytestMode: define<boolean>('we:exitPlaytestMode'),

  EnvironmentSet: define<WESetEnvironmentRequest>('we:setEnvironment'),
  EnvironmentAck: define<WEAckEnvironmentRequest>('we:ackEnvironment'),

  SettingsSet: define<WESettingsChangeRequest>('we:settings'),

  AdditionCreate: define<WECreateAdditionRequest>(worldEditorApi.createAddition),
  AdditionSet: define<WESetAdditionRequest>(worldEditorApi.setAddition),
  AdditionApplyChange: define<WEApplyAdditionChangeRequest>(worldEditorApi.applyAdditionChange),
  AdditionSetOnGround: define<string>('we:setAdditionOnGround'),
  AdditionDelete: define<string>(worldEditorApi.deleteAddition),
  AdditionDeleteBatch: define<string[]>('we:deleteAdditions'),

  PatchCreate: define<WECreatePatchRequest>(worldEditorApi.createPatch),
  PatchApplyChange: define<WEApplyPatchChangeRequest>(worldEditorApi.applyPatchChange),
  PatchDelete: define<WEDeletePatchRequest>(worldEditorApi.deletePatch),
});


type TWEApi = typeof WEApi;

export type WEApiMethod = TWEApi[keyof TWEApi];
export type WEApiMethodRequest<Method extends MTR<any>> = Method['__rq'];

function define<Request>(method: string): WEApiMethodDefinition<Request> {
  return method as any;
}

type MTR<T> = { __rq: T };
type WEApiMethodDefinition<R> = string & MTR<R>;




// exampleUsage(WEApi.SELECTION, { type: WESelectionType.ADDITION,  })

// function exampleUsage<Method extends WEApiMethod>(method: Method, request: WEApiMethodRequest<Method>) { }
