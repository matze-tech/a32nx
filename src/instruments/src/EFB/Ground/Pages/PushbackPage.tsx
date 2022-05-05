// Copyright (c) 2022 FlyByWire Simulations
// SPDX-License-Identifier: GPL-3.0

/* eslint-disable max-len */
import React, { useEffect, useRef } from 'react';
import { useSimVar, useSplitSimVar } from '@instruments/common/simVars';
import {
    ArrowDown,
    ArrowLeft,
    ArrowRight, ArrowsAngleContract, ArrowsAngleExpand,
    ArrowUp,
    ChevronDoubleDown,
    ChevronDoubleUp,
    ChevronLeft,
    ChevronRight, DashCircle, DashCircleFill,
    PauseCircleFill,
    PlayCircleFill, ToggleOff, ToggleOn,
    TruckFlatbed,
} from 'react-bootstrap-icons';
import Slider from 'rc-slider';
import { MathUtils } from '@shared/MathUtils';
import { toast } from 'react-toastify';
import { t } from '../../translation';
import { TooltipWrapper } from '../../UtilComponents/TooltipWrapper';
import { useAppDispatch, useAppSelector } from '../../Store/store';
import {
    setActualMapLatLon, setAircraftIconPosition,
    setPushbackPaused,
    setShowDebugInfo,
    setTugCommandedHeadingFactor,
    setTugCommandedSpeedFactor,
    setTugInertiaFactor,
} from '../../Store/features/pushback';
import { PromptModal, useModals } from '../../UtilComponents/Modals/Modals';
import { PushbackMap } from './PushbackMap';

export const PushbackPage = () => {
    const dispatch = useAppDispatch();
    const { showModal } = useModals();

    // This is used to completely turn off the pushback for compatible with other
    // pushback add-ons. Only watching sim variable like PUSHBACK STATE or
    // Pushback Available lead to conflicts as other add-on also write to them.
    // It is implemented as a LVAR to allow 3rd parties to deactivate it if required.
    const [pushbackSystemEnabled, setPushbackSystemEnabled] = useSimVar('L:A32NX_PUSHBACK_SYSTEM_ENABLED', 'bool', 100);

    const [planeLatitude] = useSimVar('A:PLANE LATITUDE', 'degrees latitude', 50);
    const [planeLongitude] = useSimVar('A:PLANE LONGITUDE', 'degrees longitude', 50);

    const [pushbackAttached] = useSimVar('Pushback Attached', 'bool', 100);
    const [pushbackState, setPushbackState] = useSplitSimVar('PUSHBACK STATE', 'enum', 'K:TOGGLE_PUSHBACK', 'bool', 250);
    const [pushbackWait, setPushbackWait] = useSimVar('Pushback Wait', 'bool', 100);
    const [pushbackAngle] = useSimVar('PUSHBACK ANGLE', 'Radians', 100);

    const [rudderPosition] = useSimVar('A:RUDDER POSITION', 'number', 50);
    const [elevatorPosition] = useSimVar('A:ELEVATOR POSITION', 'number', 50);

    const [planeGroundSpeed] = useSimVar('GROUND VELOCITY', 'Knots', 50);
    const [planeHeadingTrue] = useSimVar('PLANE HEADING DEGREES TRUE', 'degrees', 50);
    const [planeHeadingMagnetic] = useSimVar('PLANE HEADING DEGREES MAGNETIC', 'degrees', 50);

    const [parkingBrakeEngaged, setParkingBrakeEngaged] = useSimVar('L:A32NX_PARK_BRAKE_LEVER_POS', 'Bool', 250);

    // Reducer state for pushback
    const {
        pushbackPaused,
        updateDeltaTime,
        showDebugInfo,
        tugCommandedHeadingFactor,
        tugCommandedHeading,
        tugCommandedSpeedFactor,
        tugCommandedSpeed,
        tugInertiaFactor,
    } = useAppSelector((state) => state.pushback.pushbackState);

    // Required so these can be used inside the useEffect return callback
    const pushBackAttachedRef = useRef(pushbackAttached);
    pushBackAttachedRef.current = pushbackAttached;
    const pushbackPausedRef = useRef(pushbackPaused);
    pushbackPausedRef.current = pushbackPaused;

    const handleEnableSystem = () => {
        if (pushbackSystemEnabled) {
            if (pushbackState < 3) {
                showModal(
                    <PromptModal
                        title={t('Pushback.DisableSystemMessageTitle')}
                        bodyText={`${t('Pushback.DisableSystemMessageBody')}`}
                        onConfirm={() => {
                            setPushbackState(!pushbackState);
                            setPushbackSystemEnabled(0);
                        }}
                    />,
                );
            } else {
                setPushbackSystemEnabled(0);
            }
            return;
        }
        showModal(
            <PromptModal
                title={t('Pushback.EnableSystemMessageTitle')}
                bodyText={`${t('Pushback.EnableSystemMessageBody')}`}
                onConfirm={() => {
                    setPushbackSystemEnabled(1);
                }}
            />,
        );
    };

    const handleCallTug = () => {
        setPushbackState(!pushbackState);
        setPushbackWait(1);
    };

    const handlePause = () => {
        if (!pushbackPaused) {
            dispatch(setTugCommandedHeadingFactor(0));
            dispatch(setTugCommandedSpeedFactor(0));
        }
        dispatch(setPushbackPaused(!pushbackPaused));
    };

    const handleTugSpeed = (speed: number) => {
        dispatch(setTugCommandedSpeedFactor(MathUtils.clamp(speed, -1, 1)));
        if (speed) {
            dispatch(setPushbackPaused(false));
        }
    };

    const handleTugDirection = (value: number) => {
        dispatch(setTugCommandedHeadingFactor(MathUtils.clamp(value, -1, 1)));
    };

    const pushbackActive = () => pushbackSystemEnabled && pushbackAttached;

    // FIXME
    // const decelerateTug = (factor: number = 1) => {
    //     const r = 0.05;
    //     const bf = factor;
    //     dispatch(setTugInertiaFactor(bf));
    //     if (bf <= 0) {
    //         console.log('Decelerated!');
    //         return;
    //     }
    //     setTimeout(() => {
    //         decelerateTug(bf - r);
    //     }, 50);
    // };
    //
    // const accelerateTug = (factor: number = 0) => {
    //     const r = 0.05;
    //     const bf = factor;
    //     dispatch(setTugInertiaFactor(bf));
    //     if (bf >= 1) {
    //         console.log('Accelerated!');
    //         return;
    //     }
    //     setTimeout(() => {
    //         accelerateTug(bf + r);
    //     }, 50);
    // };

    // called once when loading and unloading the page
    useEffect(() => {
        // when loading the page
        dispatch(setPushbackPaused(true));

        // when unloading the page
        // !obs: as with setInterval no access to current local variable values
        return (() => {
            if (pushBackAttachedRef.current && !pushbackPausedRef.current) {
                toast.info(t('Pushback.LeavePageMessage'), {
                    autoClose: 750,
                    hideProgressBar: true,
                    closeButton: false,
                });
                dispatch(setPushbackPaused(true));
            }
        });
    }, []);

    // Update commanded heading from rudder input
    useEffect(() => {
        if (!pushbackActive()) {
            return;
        }
        // create deadzone
        if (rudderPosition > -0.05 && rudderPosition < 0.05) {
            dispatch(setTugCommandedHeadingFactor(0));
            return;
        }
        dispatch(setTugCommandedHeadingFactor(rudderPosition));
    }, [rudderPosition]);

    // Update commanded speed from elevator input
    useEffect(() => {
        if (!pushbackActive()) {
            return;
        }
        // create deadzone
        if (elevatorPosition > -0.05 && elevatorPosition < 0.05) {
            dispatch(setTugCommandedSpeedFactor(0));
            return;
        }
        dispatch(setPushbackPaused(false));
        dispatch(setTugCommandedSpeedFactor(-elevatorPosition));
    }, [elevatorPosition]);

    // Debug info for pushback movement - can be removed eventually
    const debugInformation = () => (
        <div className="flex absolute right-0 left-0 z-50 flex-grow justify-between mx-4 font-mono text-black bg-gray-100 border-gray-100 opacity-50">
            <div className="overflow-hidden text-black text-m">
                deltaTime:
                {' '}
                {updateDeltaTime}
                <br />
                pushbackPaused:
                {' '}
                {pushbackPaused ? 1 : 0}
                <br />
                pushBackWait:
                {' '}
                {pushbackWait}
                <br />
                pushBackAttached:
                {' '}
                {pushbackAttached}
                <br />
                pushBackState:
                {' '}
                {pushbackState}
                <br />
                pushbackAvailable:
                {' '}
                {SimVar.GetSimVarValue('PUSHBACK AVAILABLE', 'bool')}
                <br />
                tugAngle:
                {' '}
                {pushbackAngle.toFixed(3)}
                {' ('}
                {(pushbackAngle * (180 / Math.PI)).toFixed(3)}
                {' °)'}
                <br />
                Heading (True):
                {' '}
                {planeHeadingTrue.toFixed(4)}
                <br />
                Heading (Magnetic):
                {' '}
                {planeHeadingMagnetic.toFixed(4)}
            </div>
            <div className="overflow-hidden text-black text-m">
                acHeading:
                {' '}
                {planeHeadingTrue.toFixed(3)}
                <br />
                tCHeadingF:
                {' '}
                {tugCommandedHeadingFactor.toFixed(3)}
                <br />
                tCHeading :
                {' '}
                {tugCommandedHeading.toFixed(3)}
                <br />
                Rotation Velocity X:
                {' '}
                {SimVar.GetSimVarValue('ROTATION VELOCITY BODY Y', 'Number').toFixed(3)}
                <br />
                Rotation Velocity Y:
                {' '}
                {SimVar.GetSimVarValue('ROTATION VELOCITY BODY X', 'Number').toFixed(3)}
                <br />
                {' '}
                Rotation Velocity Z:
                {' '}
                {SimVar.GetSimVarValue('ROTATION VELOCITY BODY Z', 'Number').toFixed(3)}
                <br />
                {' '}
                Rot. Accel. X:
                {' '}
                {SimVar.GetSimVarValue('ROTATION ACCELERATION BODY X', 'radians per second squared').toFixed(3)}
                <br />
                {' '}
                Rot. Accel. Y:
                {' '}
                {SimVar.GetSimVarValue('ROTATION ACCELERATION BODY Y', 'radians per second squared').toFixed(3)}
                <br />
                {' '}
                Rot. Accel Z:
                {' '}
                {SimVar.GetSimVarValue('ROTATION ACCELERATION BODY Z', 'radians per second squared').toFixed(3)}
            </div>
            <div className="overflow-hidden text-black text-m">
                acGroundSpeed:
                {' '}
                {planeGroundSpeed.toFixed(3)}
                {'kts '}
                {' ('}
                {(planeGroundSpeed * 1.68781).toFixed(3)}
                ft/s)
                <br />
                tugInertiaFactor:
                {' '}
                {tugInertiaFactor.toFixed(2)}
                <br />
                tCSpeedFactor:
                {' '}
                {tugCommandedSpeedFactor.toFixed(3)}
                <br />
                tCSpeed:
                {' '}
                {tugCommandedSpeed.toFixed(3)}
                <br />
                Velocity X:
                {' '}
                {SimVar.GetSimVarValue('VELOCITY BODY Y', 'Number').toFixed(3)}
                <br />
                Velocity Y:
                {' '}
                {SimVar.GetSimVarValue('VELOCITY BODY X', 'Number').toFixed(3)}
                <br />
                Velocity Z:
                {' '}
                {SimVar.GetSimVarValue('VELOCITY BODY Z', 'Number').toFixed(3)}
                <br />
                {' '}
                Accel. X:
                {' '}
                {SimVar.GetSimVarValue('ACCELERATION BODY X', 'feet per second squared').toFixed(3)}
                <br />
                {' '}
                Accel. Y:
                {' '}
                {SimVar.GetSimVarValue('ACCELERATION BODY Y', 'feet per second squared').toFixed(3)}
                <br />
                {' '}
                Accel Z:
                {' '}
                {SimVar.GetSimVarValue('ACCELERATION BODY Z', 'feet per second squared').toFixed(3)}

            </div>
        </div>
    );

    return (
        <div className="flex relative flex-col space-y-4">

            {/* Map Container */}
            <div className="flex relative flex-col space-y-4 h-[430px]">
                <PushbackMap />

                {/* Pushback Debug Information */}
                {showDebugInfo && debugInformation()}
            </div>

            {/* Manual Pushback Controls */}
            <div className="flex flex-col p-6 space-y-4 h-1/3 rounded-lg border-2 border-theme-accent">
                <div className="flex flex-row space-x-4">
                    {/* Pushback System enabled On/Off */}
                    {pushbackSystemEnabled ? (
                        <div className="w-full">
                            <p className="text-center">{t('Pushback.SystemEnabledOn')}</p>
                            <TooltipWrapper text={t('Pushback.TT.SystemEnabledOn')}>
                                <button
                                    type="button"
                                    onClick={handleEnableSystem}
                                    className="flex justify-center items-center w-full h-20 text-theme-text bg-green-600 rounded-md border-2 border-theme-accent opacity-60 hover:opacity-100 transition duration-100"
                                >
                                    <ToggleOn size={50} />
                                </button>
                            </TooltipWrapper>
                        </div>
                    ) : (
                        <div className="w-full">
                            <p className="text-center">{t('Pushback.SystemEnabledOff')}</p>
                            <TooltipWrapper text={t('Pushback.TT.SystemEnabledOff')}>
                                <button
                                    type="button"
                                    onClick={handleEnableSystem}
                                    className="flex justify-center items-center w-full h-20 text-theme-text bg-red-600 rounded-md border-2 border-theme-accent opacity-60 hover:opacity-100 transition duration-100"
                                >
                                    <ToggleOff size={50} />
                                </button>
                            </TooltipWrapper>
                        </div>
                    )}

                    {/* Call Tug */}
                    <div className="w-full">
                        <p className={`text-center ${!pushbackSystemEnabled && 'opacity-30 pointer-events-none'}`}>
                            {pushbackActive() ? t('Pushback.TugAttached') : t('Pushback.CallTug')}
                        </p>
                        <TooltipWrapper text={t('Pushback.TT.CallReleaseTug')}>
                            <button
                                type="button"
                                onClick={handleCallTug}
                                className={`flex justify-center items-center w-full h-20 text-theme-text bg-green-600 rounded-md border-2 border-theme-accent opacity-60 hover:opacity-100 transition duration-100'} ${!pushbackSystemEnabled && 'opacity-30 pointer-events-none'}`}
                            >
                                <TruckFlatbed size={50} />
                                {pushbackActive() ? (
                                    <ArrowsAngleContract className="ml-4" size={40} />
                                ) : (
                                    <ArrowsAngleExpand className="ml-4" size={40} />
                                )}
                            </button>
                        </TooltipWrapper>
                    </div>

                    {/* Parking Brake */}
                    <div className="w-full">
                        <p className="text-center jus">
                            {t('Pushback.ParkingBrake.Title')}
                            {' '}
                            {parkingBrakeEngaged ? t('Pushback.ParkingBrake.On') : t('Pushback.ParkingBrake.Off')}
                        </p>
                        <TooltipWrapper text={t('Pushback.TT.SetReleaseParkingBrake')}>
                            <button
                                type="button"
                                onClick={() => setParkingBrakeEngaged((old) => !old)}
                                className={`w-full h-20 rounded-md transition duration-100 flex items-center justify-center text-utility-white opacity-60 hover:opacity-100  ${parkingBrakeEngaged ? 'bg-red-600' : 'bg-green-600'}`}
                            >
                                {parkingBrakeEngaged ? (
                                    <DashCircleFill className="transform" size={40} />
                                ) : (
                                    <DashCircle className="transform -rotate-90" size={40} />
                                )}
                            </button>
                        </TooltipWrapper>
                    </div>
                </div>

                <div className="flex flex-row space-x-4">

                    {/* Backward Button */}
                    <div className="w-full">
                        <p className={`text-center ${!pushbackActive() && 'opacity-30 pointer-events-none'}`}>
                            { t('Pushback.Backward') }
                        </p>
                        <TooltipWrapper text={t('Pushback.TT.DecreaseSpeed')}>
                            <button
                                type="button"
                                className={`flex justify-center items-center w-full h-20 bg-theme-highlight hover:bg-theme-body rounded-md border-2 border-theme-highlight transition duration-100 hover:text-theme-highlight ${!pushbackActive() && 'opacity-30 pointer-events-none'}`}
                                onClick={() => handleTugSpeed(tugCommandedSpeedFactor - 0.1)}
                            >
                                <ArrowDown size={40} />
                            </button>
                        </TooltipWrapper>
                    </div>

                    {/* Forward Button */}
                    <div className="w-full">
                        <p className={`text-center ${!pushbackActive() && 'opacity-30 pointer-events-none'}`}>
                            {t('Pushback.Forward')}
                        </p>
                        <TooltipWrapper text={t('Pushback.TT.IncreaseSpeed')}>
                            <button
                                type="button"
                                className={`flex justify-center items-center w-full h-20 bg-theme-highlight hover:bg-theme-body rounded-md border-2 border-theme-highlight transition duration-100 hover:text-theme-highlight ${!pushbackActive() && 'opacity-30 pointer-events-none'}`}
                                onClick={() => handleTugSpeed(tugCommandedSpeedFactor + 0.1)}
                            >
                                <ArrowUp size={40} />
                            </button>
                        </TooltipWrapper>
                    </div>

                    {/* Pause/Moving Button */}
                    <div className="w-full">
                        <p className="text-center">
                            {pushbackPaused ? t('Pushback.Halt') : t('Pushback.Moving')}
                        </p>
                        <TooltipWrapper text={t('Pushback.TT.PausePushback')}>
                            <button
                                type="button"
                                onClick={handlePause}
                                className={`flex justify-center items-center w-full h-20 bg-theme-highlight hover:bg-theme-body rounded-md border-2 border-theme-highlight transition duration-100 hover:text-theme-highlight ${!pushbackActive() && 'opacity-30 pointer-events-none'}`}
                            >
                                {pushbackPaused ? (
                                    <PlayCircleFill size={40} />
                                ) : (
                                    <PauseCircleFill size={40} />
                                )}
                            </button>
                        </TooltipWrapper>
                    </div>

                    {/* Left Button */}
                    <div className="w-full">
                        <p className={`text-center ${!pushbackActive() && 'opacity-30 pointer-events-none'}`}>
                            {t('Pushback.Left')}
                        </p>
                        <TooltipWrapper text={t('Pushback.TT.Left')}>
                            <button
                                type="button"
                                className={`flex justify-center items-center w-full h-20 bg-theme-highlight hover:bg-theme-body rounded-md border-2 border-theme-highlight transition duration-100 hover:text-theme-highlight ${!pushbackActive() && 'opacity-30 pointer-events-none'}`}
                                onClick={() => handleTugDirection(tugCommandedHeadingFactor - 0.1)}
                            >
                                <ArrowLeft size={40} />
                            </button>
                        </TooltipWrapper>
                    </div>

                    {/* Right Button */}
                    <div className="w-full">
                        <p className={`text-center ${!pushbackActive() && 'opacity-30 pointer-events-none'}`}>
                            {t('Pushback.Right')}
                        </p>
                        <TooltipWrapper text={t('Pushback.TT.Right')}>
                            <button
                                type="button"
                                className={`flex justify-center items-center w-full h-20 bg-theme-highlight hover:bg-theme-body rounded-md border-2 border-theme-highlight transition duration-100 hover:text-theme-highlight ${!pushbackActive() && 'opacity-30 pointer-events-none'}`}
                                onClick={() => handleTugDirection(tugCommandedHeadingFactor + 0.1)}
                            >
                                <ArrowRight size={40} />
                            </button>
                        </TooltipWrapper>
                    </div>
                </div>

                {/* Direction Slider */}
                <div>
                    <p className={`text-center ${!pushbackActive() && 'opacity-30 pointer-events-none'}`}>
                        {t('Pushback.TugDirection')}
                    </p>
                    <TooltipWrapper text={t('Pushback.TT.SliderDirection')}>
                        <div className="flex flex-row items-center space-x-4">
                            <p className="font-bold text-unselected"><ChevronLeft /></p>
                            <Slider
                                className={`${!pushbackActive() && 'opacity-30 pointer-events-none'}`}
                                onChange={(value) => handleTugDirection(value)}
                                min={-1}
                                step={0.01}
                                max={1}
                                value={tugCommandedHeadingFactor}
                                startPoint={0}
                            />
                            <p className="font-bold text-unselected"><ChevronRight /></p>
                        </div>
                    </TooltipWrapper>
                </div>

                {/* Speed Slider */}
                <div>
                    <p className={`text-center ${!pushbackActive() && 'opacity-30 pointer-events-none'}`}>
                        {t('Pushback.TugSpeed')}
                    </p>
                    <TooltipWrapper text={t('Pushback.TT.SliderSpeed')}>
                        <div className="flex flex-row items-center space-x-4">
                            <p className="font-bold text-unselected"><ChevronDoubleDown /></p>
                            <Slider
                                className={`${!pushbackActive() && 'opacity-30 pointer-events-none'}`}
                                min={-1}
                                step={0.1}
                                max={1}
                                value={tugCommandedSpeedFactor}
                                onChange={(value) => handleTugSpeed(value)}
                                startPoint={0}
                            />
                            <p
                                className="font-bold text-unselected"
                                onDoubleClick={() => dispatch(setShowDebugInfo(!showDebugInfo))}
                            >
                                <ChevronDoubleUp />
                            </p>
                        </div>
                    </TooltipWrapper>
                </div>
            </div>
        </div>
    );
};
