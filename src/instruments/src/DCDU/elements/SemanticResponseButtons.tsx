import React from 'react';
import { AtsuMessageComStatus } from '@atsu/messages/AtsuMessage';
import { CpdlcMessage } from '@atsu/messages/CpdlcMessage';
import { UplinkMessageInterpretation } from '@atsu/components/UplinkMessageInterpretation';
import { Button } from './Button';

type SemanticResponseButtonsProps = {
    message: CpdlcMessage,
    dataIncomplete: boolean,
    invertResponse: (message: number) => void,
    sendMessage: (message: number) => void,
    closeMessage: (message: number) => void
}

export const SemanticResponseButtons: React.FC<SemanticResponseButtonsProps> = ({ message, dataIncomplete, invertResponse, sendMessage, closeMessage }) => {
    const showAnswers = message.Response === undefined || (message.Response.ComStatus !== AtsuMessageComStatus.Sending && message.Response.ComStatus !== AtsuMessageComStatus.Sent);
    const buttonsBlocked = message.Response !== undefined && message.Response.ComStatus === AtsuMessageComStatus.Sending;

    const clicked = (index: string) : void => {
        if (message.UniqueMessageID === undefined || buttonsBlocked) {
            return;
        }

        if (showAnswers) {
            if (index === 'L1') {
                invertResponse(message.UniqueMessageID);
            } else if (index === 'R2' && message.Response) {
                sendMessage(message.Response.UniqueMessageID);
            }
            // TODO process R1 and modify the message
        } else if (index === 'R2') {
            closeMessage(message.UniqueMessageID);
        }
    };

    return (
        <>
            {showAnswers && (
                <>
                    {UplinkMessageInterpretation.HasNegativeResponse(message) && (
                        <>
                            <Button
                                messageId={message.UniqueMessageID}
                                index="L1"
                                content="CANNOT"
                                active={!buttonsBlocked && !dataIncomplete}
                                onClick={clicked}
                            />
                        </>
                    )}
                    <Button
                        messageId={message.UniqueMessageID}
                        index="R1"
                        content="MODIFY"
                        active={!buttonsBlocked}
                        onClick={clicked}
                    />
                    <Button
                        messageId={message.UniqueMessageID}
                        index="R2"
                        content="SEND"
                        active={!dataIncomplete}
                        onClick={clicked}
                    />
                </>
            )}
            {!showAnswers && (
                <Button
                    messageId={message.UniqueMessageID}
                    index="R2"
                    content="CLOSE"
                    active={!buttonsBlocked}
                    onClick={clicked}
                />
            )}
        </>
    );
};
