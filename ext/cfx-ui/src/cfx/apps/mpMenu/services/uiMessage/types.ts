import { IFormattedMessage } from '../../utils/messageFormatting';

export interface IUiMessage extends IFormattedMessage {
  type: 'info' | 'warning';
}
