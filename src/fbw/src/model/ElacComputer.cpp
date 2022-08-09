#include "ElacComputer.h"
#include "ElacComputer_types.h"
#include "rtwtypes.h"
#include <cmath>
#include "look2_binlxpw.h"
#include "look1_binlxpw.h"
#include "LateralNormalLaw.h"
#include "LateralDirectLaw.h"
#include "PitchNormalLaw.h"
#include "PitchAlternateLaw.h"
#include "PitchDirectLaw.h"

const uint8_T ElacComputer_IN_Flight{ 1U };

const uint8_T ElacComputer_IN_FlightToGroundTransition{ 2U };

const uint8_T ElacComputer_IN_Ground{ 3U };

const uint8_T ElacComputer_IN_NO_ACTIVE_CHILD{ 0U };

const uint8_T ElacComputer_IN_Flying{ 1U };

const uint8_T ElacComputer_IN_Landed{ 2U };

const uint8_T ElacComputer_IN_Landing100ft{ 3U };

const uint8_T ElacComputer_IN_Takeoff100ft{ 4U };

const real_T ElacComputer_RGND{ 0.0 };

void ElacComputer::ElacComputer_MATLABFunction(const base_arinc_429 *rtu_u, boolean_T *rty_y)
{
  *rty_y = (rtu_u->SSM != static_cast<uint32_T>(SignStatusMatrix::FailureWarning));
}

void ElacComputer::ElacComputer_MATLABFunction_j(const base_arinc_429 *rtu_u, real_T rtu_bit, uint32_T *rty_y)
{
  real32_T tmp;
  uint32_T a;
  tmp = std::round(rtu_u->Data);
  if (tmp < 4.2949673E+9F) {
    if (tmp >= 0.0F) {
      a = static_cast<uint32_T>(tmp);
    } else {
      a = 0U;
    }
  } else {
    a = MAX_uint32_T;
  }

  if (-(rtu_bit - 1.0) >= 0.0) {
    if (-(rtu_bit - 1.0) <= 31.0) {
      a <<= static_cast<uint8_T>(-(rtu_bit - 1.0));
    } else {
      a = 0U;
    }
  } else if (-(rtu_bit - 1.0) >= -31.0) {
    a >>= static_cast<uint8_T>(rtu_bit - 1.0);
  } else {
    a = 0U;
  }

  *rty_y = a & 1U;
}

void ElacComputer::ElacComputer_RateLimiter_Reset(rtDW_RateLimiter_ElacComputer_T *localDW)
{
  localDW->pY_not_empty = false;
}

void ElacComputer::ElacComputer_RateLimiter(real_T rtu_u, real_T rtu_up, real_T rtu_lo, real_T rtu_Ts, real_T rtu_init,
  real_T *rty_Y, rtDW_RateLimiter_ElacComputer_T *localDW)
{
  if (!localDW->pY_not_empty) {
    localDW->pY = rtu_init;
    localDW->pY_not_empty = true;
  }

  localDW->pY += std::fmax(std::fmin(rtu_u - localDW->pY, std::abs(rtu_up) * rtu_Ts), -std::abs(rtu_lo) * rtu_Ts);
  *rty_Y = localDW->pY;
}

void ElacComputer::ElacComputer_RateLimiter_o_Reset(rtDW_RateLimiter_ElacComputer_g_T *localDW)
{
  localDW->pY_not_empty = false;
}

void ElacComputer::ElacComputer_RateLimiter_a(real_T rtu_u, real_T rtu_up, real_T rtu_lo, real_T rtu_Ts, real_T rtu_init,
  boolean_T rtu_reset, real_T *rty_Y, rtDW_RateLimiter_ElacComputer_g_T *localDW)
{
  if ((!localDW->pY_not_empty) || rtu_reset) {
    localDW->pY = rtu_init;
    localDW->pY_not_empty = true;
  }

  if (rtu_reset) {
    *rty_Y = rtu_init;
  } else {
    *rty_Y = std::fmax(std::fmin(rtu_u - localDW->pY, std::abs(rtu_up) * rtu_Ts), -std::abs(rtu_lo) * rtu_Ts) +
      localDW->pY;
  }

  localDW->pY = *rty_Y;
}

void ElacComputer::ElacComputer_MATLABFunction_o(boolean_T rtu_bit1, boolean_T rtu_bit2, boolean_T rtu_bit3, boolean_T
  rtu_bit4, boolean_T rtu_bit5, boolean_T rtu_bit6, real_T *rty_handleIndex)
{
  if (rtu_bit1) {
    *rty_handleIndex = 0.0;
  } else if (rtu_bit2 && rtu_bit6) {
    *rty_handleIndex = 1.0;
  } else if (rtu_bit2 && (!rtu_bit6)) {
    *rty_handleIndex = 2.0;
  } else if (rtu_bit3) {
    *rty_handleIndex = 3.0;
  } else if (rtu_bit4) {
    *rty_handleIndex = 4.0;
  } else if (rtu_bit5) {
    *rty_handleIndex = 5.0;
  } else {
    *rty_handleIndex = 0.0;
  }
}

void ElacComputer::ElacComputer_LagFilter_Reset(rtDW_LagFilter_ElacComputer_T *localDW)
{
  localDW->pY_not_empty = false;
  localDW->pU_not_empty = false;
}

void ElacComputer::ElacComputer_LagFilter(real_T rtu_U, real_T rtu_C1, real_T rtu_dt, real_T *rty_Y,
  rtDW_LagFilter_ElacComputer_T *localDW)
{
  real_T ca;
  real_T denom_tmp;
  if ((!localDW->pY_not_empty) || (!localDW->pU_not_empty)) {
    localDW->pU = rtu_U;
    localDW->pU_not_empty = true;
    localDW->pY = rtu_U;
    localDW->pY_not_empty = true;
  }

  denom_tmp = rtu_dt * rtu_C1;
  ca = denom_tmp / (denom_tmp + 2.0);
  *rty_Y = (2.0 - denom_tmp) / (denom_tmp + 2.0) * localDW->pY + (rtu_U * ca + localDW->pU * ca);
  localDW->pY = *rty_Y;
  localDW->pU = rtu_U;
}

void ElacComputer::ElacComputer_MATLABFunction_g5_Reset(rtDW_MATLABFunction_ElacComputer_kz_T *localDW)
{
  localDW->output = false;
  localDW->timeSinceCondition = 0.0;
}

void ElacComputer::ElacComputer_MATLABFunction_c(boolean_T rtu_u, real_T rtu_Ts, boolean_T rtu_isRisingEdge, real_T
  rtu_timeDelay, boolean_T *rty_y, rtDW_MATLABFunction_ElacComputer_kz_T *localDW)
{
  if (rtu_u == rtu_isRisingEdge) {
    localDW->timeSinceCondition += rtu_Ts;
    if (localDW->timeSinceCondition >= rtu_timeDelay) {
      localDW->output = rtu_u;
    }
  } else {
    localDW->timeSinceCondition = 0.0;
    localDW->output = rtu_u;
  }

  *rty_y = localDW->output;
}

void ElacComputer::ElacComputer_MATLABFunction_k_Reset(rtDW_MATLABFunction_ElacComputer_o_T *localDW)
{
  localDW->output = false;
}

void ElacComputer::ElacComputer_MATLABFunction_m(real_T rtu_u, real_T rtu_highTrigger, real_T rtu_lowTrigger, boolean_T *
  rty_y, rtDW_MATLABFunction_ElacComputer_o_T *localDW)
{
  boolean_T output_tmp;
  output_tmp = !localDW->output;
  localDW->output = ((output_tmp && (rtu_u >= rtu_highTrigger)) || ((output_tmp || (rtu_u > rtu_lowTrigger)) &&
    localDW->output));
  *rty_y = localDW->output;
}

void ElacComputer::ElacComputer_GetIASforMach4(real_T rtu_m, real_T rtu_m_t, real_T rtu_v, real_T *rty_v_t)
{
  *rty_v_t = rtu_v * rtu_m_t / rtu_m;
}

void ElacComputer::ElacComputer_RateLimiter_d_Reset(rtDW_RateLimiter_ElacComputer_b_T *localDW)
{
  localDW->pY_not_empty = false;
}

void ElacComputer::ElacComputer_RateLimiter_n(real_T rtu_u, real_T rtu_up, real_T rtu_lo, real_T rtu_Ts, boolean_T
  rtu_reset, real_T *rty_Y, rtDW_RateLimiter_ElacComputer_b_T *localDW)
{
  if ((!localDW->pY_not_empty) || rtu_reset) {
    localDW->pY = rtu_u;
    localDW->pY_not_empty = true;
  }

  if (rtu_reset) {
    *rty_Y = rtu_u;
  } else {
    *rty_Y = std::fmax(std::fmin(rtu_u - localDW->pY, std::abs(rtu_up) * rtu_Ts), -std::abs(rtu_lo) * rtu_Ts) +
      localDW->pY;
  }

  localDW->pY = *rty_Y;
}

void ElacComputer::ElacComputer_MATLABFunction_ax_Reset(rtDW_MATLABFunction_ElacComputer_b_T *localDW)
{
  localDW->previousInput_not_empty = false;
}

void ElacComputer::ElacComputer_MATLABFunction_g(boolean_T rtu_u, boolean_T rtu_isRisingEdge, boolean_T *rty_y,
  rtDW_MATLABFunction_ElacComputer_b_T *localDW)
{
  if (!localDW->previousInput_not_empty) {
    localDW->previousInput = rtu_isRisingEdge;
    localDW->previousInput_not_empty = true;
  }

  if (rtu_isRisingEdge) {
    *rty_y = (rtu_u && (!localDW->previousInput));
  } else {
    *rty_y = ((!rtu_u) && localDW->previousInput);
  }

  localDW->previousInput = rtu_u;
}

void ElacComputer::ElacComputer_MATLABFunction_cw(const boolean_T rtu_u[19], real32_T *rty_y)
{
  uint32_T out;
  out = 0U;
  for (int32_T i{0}; i < 19; i++) {
    out |= static_cast<uint32_T>(rtu_u[i]) << (i + 10);
  }

  *rty_y = static_cast<real32_T>(out);
}

void ElacComputer::ElacComputer_LateralLawCaptoBits(lateral_efcs_law rtu_law, boolean_T *rty_bit1, boolean_T *rty_bit2)
{
  *rty_bit1 = (rtu_law == lateral_efcs_law::NormalLaw);
  *rty_bit2 = (rtu_law == lateral_efcs_law::DirectLaw);
}

void ElacComputer::step()
{
  real_T rtb_xi_deg;
  real_T rtb_zeta_deg;
  real_T rtb_eta_deg;
  real_T rtb_eta_trim_dot_deg_s;
  real_T rtb_eta_trim_limit_lo;
  real_T rtb_eta_trim_limit_up;
  real_T rtb_eta_deg_o;
  real_T rtb_eta_trim_dot_deg_s_a;
  real_T rtb_eta_trim_limit_lo_h;
  real_T rtb_eta_trim_limit_up_d;
  base_arinc_429 rtb_BusConversion_InsertedFor_MATLABFunction_at_inport_0_BusCreator1_hi;
  real_T rtb_DataTypeConversion3_m;
  real_T rtb_DataTypeConversion8;
  real_T rtb_Switch_b;
  real_T rtb_Y;
  real_T rtb_Y_b;
  real_T rtb_Y_c;
  real_T rtb_eta_trim_limit_lo_d;
  real_T rtb_handleIndex;
  real32_T rtb_mach_h;
  uint32_T rtb_Switch18;
  uint32_T rtb_y_b;
  uint32_T rtb_y_e;
  uint32_T rtb_y_o;
  boolean_T rtb_VectorConcatenate[19];
  boolean_T rtb_VectorConcatenate_a[19];
  boolean_T rtb_AND2_p;
  boolean_T rtb_AND_ai;
  boolean_T rtb_NOT_k;
  boolean_T rtb_doubleAdrFault;
  boolean_T rtb_doubleIrFault;
  boolean_T rtb_tripleAdrFault;
  boolean_T rtb_y_i;
  if (ElacComputer_U.in.sim_data.computer_running) {
    real_T rtb_DataTypeConversion8_g;
    real_T rtb_logic_crg14_total_sidestick_roll_command;
    int32_T rtb_ap_special_disc;
    real32_T rtb_V_ias;
    real32_T rtb_V_tas;
    real32_T rtb_alpha;
    real32_T rtb_n_x;
    real32_T rtb_n_y;
    real32_T rtb_n_z;
    real32_T rtb_phi;
    real32_T rtb_phi_dot;
    real32_T rtb_q;
    real32_T rtb_r;
    real32_T rtb_raComputationValue;
    real32_T rtb_theta_dot;
    boolean_T alternate1Condition_tmp;
    boolean_T alternate2Condition_tmp;
    boolean_T canEngageInRoll;
    boolean_T hasPriorityInPitch;
    boolean_T hasPriorityInRoll;
    boolean_T leftAileronAvail;
    boolean_T rightAileronAvail;
    boolean_T rtb_AND1;
    boolean_T rtb_AND1_h;
    boolean_T rtb_AND2;
    boolean_T rtb_AND3_b;
    boolean_T rtb_AND4;
    boolean_T rtb_DataTypeConversion_by;
    boolean_T rtb_DataTypeConversion_cz;
    boolean_T rtb_OR;
    boolean_T rtb_OR1;
    boolean_T rtb_OR3;
    boolean_T rtb_OR4;
    boolean_T rtb_OR6;
    boolean_T rtb_OR7;
    boolean_T rtb_isEngagedInPitch;
    boolean_T rtb_isEngagedInRoll;
    boolean_T rtb_ra1Invalid;
    boolean_T rtb_thsAvail_tmp;
    boolean_T rtb_tripleIrFault;
    lateral_efcs_law priorityPitchLateralLawCap;
    lateral_efcs_law rtb_activeLateralLaw;
    lateral_efcs_law rtb_lateralLawCapability;
    lateral_efcs_law rtb_oppElacRollCapability;
    pitch_efcs_law priorityPitchPitchLawCap;
    pitch_efcs_law rtb_pitchLawCapability;
    if (!ElacComputer_DWork.Runtime_MODE) {
      ElacComputer_DWork.Delay_DSTATE_cc = ElacComputer_P.Delay_InitialCondition_c;
      ElacComputer_DWork.Delay1_DSTATE = ElacComputer_P.Delay1_InitialCondition;
      ElacComputer_DWork.Memory_PreviousInput = ElacComputer_P.SRFlipFlop_initial_condition;
      ElacComputer_DWork.Delay_DSTATE = ElacComputer_P.DiscreteDerivativeVariableTs_InitialCondition;
      ElacComputer_DWork.Delay_DSTATE_b = ElacComputer_P.Delay_InitialCondition;
      ElacComputer_DWork.icLoad = true;
      ElacComputer_LagFilter_Reset(&ElacComputer_DWork.sf_LagFilter_a);
      ElacComputer_MATLABFunction_g5_Reset(&ElacComputer_DWork.sf_MATLABFunction_jz);
      ElacComputer_MATLABFunction_g5_Reset(&ElacComputer_DWork.sf_MATLABFunction_lf);
      ElacComputer_MATLABFunction_g5_Reset(&ElacComputer_DWork.sf_MATLABFunction_jl);
      ElacComputer_DWork.ra1CoherenceRejected = false;
      ElacComputer_DWork.ra2CoherenceRejected = false;
      ElacComputer_DWork.configFullEventTime_not_empty = false;
      ElacComputer_DWork.is_active_c30_ElacComputer = 0U;
      ElacComputer_DWork.is_c30_ElacComputer = ElacComputer_IN_NO_ACTIVE_CHILD;
      ElacComputer_DWork.on_ground_time = 0.0;
      ElacComputer_B.in_flight = 0.0;
      ElacComputer_MATLABFunction_k_Reset(&ElacComputer_DWork.sf_MATLABFunction_jg);
      ElacComputer_MATLABFunction_g5_Reset(&ElacComputer_DWork.sf_MATLABFunction_cj);
      ElacComputer_MATLABFunction_k_Reset(&ElacComputer_DWork.sf_MATLABFunction_mi);
      ElacComputer_MATLABFunction_g5_Reset(&ElacComputer_DWork.sf_MATLABFunction_g2);
      ElacComputer_MATLABFunction_k_Reset(&ElacComputer_DWork.sf_MATLABFunction_br);
      ElacComputer_MATLABFunction_g5_Reset(&ElacComputer_DWork.sf_MATLABFunction_gfx);
      ElacComputer_MATLABFunction_ax_Reset(&ElacComputer_DWork.sf_MATLABFunction_g4);
      ElacComputer_MATLABFunction_ax_Reset(&ElacComputer_DWork.sf_MATLABFunction_nu);
      ElacComputer_DWork.pLeftStickDisabled = false;
      ElacComputer_DWork.pRightStickDisabled = false;
      ElacComputer_MATLABFunction_g5_Reset(&ElacComputer_DWork.sf_MATLABFunction_j2);
      ElacComputer_MATLABFunction_g5_Reset(&ElacComputer_DWork.sf_MATLABFunction_g24);
      ElacComputer_MATLABFunction_g5_Reset(&ElacComputer_DWork.sf_MATLABFunction_nb);
      ElacComputer_DWork.abnormalConditionWasActive = false;
      ElacComputer_MATLABFunction_ax_Reset(&ElacComputer_DWork.sf_MATLABFunction_l0);
      ElacComputer_DWork.eventTime_not_empty_a = false;
      ElacComputer_RateLimiter_d_Reset(&ElacComputer_DWork.sf_RateLimiter_n);
      ElacComputer_RateLimiter_d_Reset(&ElacComputer_DWork.sf_RateLimiter_m);
      ElacComputer_DWork.is_active_c28_ElacComputer = 0U;
      ElacComputer_DWork.is_c28_ElacComputer = ElacComputer_IN_NO_ACTIVE_CHILD;
      ElacComputer_DWork.eventTime_not_empty = false;
      ElacComputer_DWork.sProtActive = false;
      ElacComputer_DWork.resetEventTime_not_empty = false;
      ElacComputer_DWork.sProtActive_m = false;
      ElacComputer_RateLimiter_Reset(&ElacComputer_DWork.sf_RateLimiter);
      ElacComputer_RateLimiter_Reset(&ElacComputer_DWork.sf_RateLimiter_b);
      LawMDLOBJ2.reset();
      LawMDLOBJ1.reset();
      ElacComputer_RateLimiter_o_Reset(&ElacComputer_DWork.sf_RateLimiter_a);
      ElacComputer_RateLimiter_o_Reset(&ElacComputer_DWork.sf_RateLimiter_p);
      ElacComputer_LagFilter_Reset(&ElacComputer_DWork.sf_LagFilter);
      LawMDLOBJ5.reset();
      LawMDLOBJ3.reset();
      LawMDLOBJ4.reset();
      ElacComputer_DWork.Runtime_MODE = true;
    }

    rtb_OR1 = ((ElacComputer_U.in.bus_inputs.adr_1_bus.mach.SSM == static_cast<uint32_T>(SignStatusMatrix::
      FailureWarning)) || (ElacComputer_U.in.bus_inputs.adr_1_bus.airspeed_computed_kn.SSM == static_cast<uint32_T>
                (SignStatusMatrix::FailureWarning)) || (ElacComputer_U.in.bus_inputs.adr_1_bus.airspeed_true_kn.SSM ==
                static_cast<uint32_T>(SignStatusMatrix::FailureWarning)) ||
               (ElacComputer_U.in.bus_inputs.adr_1_bus.aoa_corrected_deg.SSM == static_cast<uint32_T>(SignStatusMatrix::
      FailureWarning)) || ElacComputer_P.Constant1_Value_b || ElacComputer_P.Constant1_Value_b);
    rtb_OR3 = ((ElacComputer_U.in.bus_inputs.adr_2_bus.mach.SSM == static_cast<uint32_T>(SignStatusMatrix::
      FailureWarning)) || (ElacComputer_U.in.bus_inputs.adr_2_bus.airspeed_computed_kn.SSM == static_cast<uint32_T>
                (SignStatusMatrix::FailureWarning)) || (ElacComputer_U.in.bus_inputs.adr_2_bus.airspeed_true_kn.SSM ==
                static_cast<uint32_T>(SignStatusMatrix::FailureWarning)) ||
               (ElacComputer_U.in.bus_inputs.adr_2_bus.aoa_corrected_deg.SSM == static_cast<uint32_T>(SignStatusMatrix::
      FailureWarning)) || ElacComputer_P.Constant1_Value_b || ElacComputer_P.Constant1_Value_b);
    rtb_OR4 = ((ElacComputer_U.in.bus_inputs.adr_3_bus.mach.SSM == static_cast<uint32_T>(SignStatusMatrix::
      FailureWarning)) || (ElacComputer_U.in.bus_inputs.adr_3_bus.airspeed_computed_kn.SSM == static_cast<uint32_T>
                (SignStatusMatrix::FailureWarning)) || (ElacComputer_U.in.bus_inputs.adr_3_bus.airspeed_true_kn.SSM ==
                static_cast<uint32_T>(SignStatusMatrix::FailureWarning)) ||
               (ElacComputer_U.in.bus_inputs.adr_3_bus.aoa_corrected_deg.SSM == static_cast<uint32_T>(SignStatusMatrix::
      FailureWarning)) || ElacComputer_P.Constant1_Value_b || ElacComputer_P.Constant1_Value_b);
    rtb_doubleAdrFault = ((rtb_OR1 && rtb_OR3) || (rtb_OR1 && rtb_OR4) || (rtb_OR3 && rtb_OR4));
    rtb_tripleAdrFault = (rtb_OR1 && rtb_OR3 && rtb_OR4);
    rtb_OR = ((ElacComputer_U.in.bus_inputs.ir_1_bus.pitch_angle_deg.SSM != static_cast<uint32_T>(SignStatusMatrix::
                NormalOperation)) || (ElacComputer_U.in.bus_inputs.ir_1_bus.roll_angle_deg.SSM != static_cast<uint32_T>
               (SignStatusMatrix::NormalOperation)) || (ElacComputer_U.in.bus_inputs.ir_1_bus.body_yaw_rate_deg_s.SSM !=
               static_cast<uint32_T>(SignStatusMatrix::NormalOperation)) ||
              (ElacComputer_U.in.bus_inputs.ir_1_bus.body_long_accel_g.SSM != static_cast<uint32_T>(SignStatusMatrix::
                NormalOperation)) || (ElacComputer_U.in.bus_inputs.ir_1_bus.body_lat_accel_g.SSM != static_cast<uint32_T>
               (SignStatusMatrix::NormalOperation)) || (ElacComputer_U.in.bus_inputs.ir_1_bus.body_normal_accel_g.SSM !=
               static_cast<uint32_T>(SignStatusMatrix::NormalOperation)) ||
              (ElacComputer_U.in.bus_inputs.ir_1_bus.pitch_att_rate_deg_s.SSM != static_cast<uint32_T>(SignStatusMatrix::
                NormalOperation)) || (ElacComputer_U.in.bus_inputs.ir_1_bus.roll_att_rate_deg_s.SSM !=
               static_cast<uint32_T>(SignStatusMatrix::NormalOperation)) || ElacComputer_P.Constant_Value_ad);
    rtb_OR6 = ((ElacComputer_U.in.bus_inputs.ir_2_bus.pitch_angle_deg.SSM != static_cast<uint32_T>(SignStatusMatrix::
      NormalOperation)) || (ElacComputer_U.in.bus_inputs.ir_2_bus.roll_angle_deg.SSM != static_cast<uint32_T>
                (SignStatusMatrix::NormalOperation)) || (ElacComputer_U.in.bus_inputs.ir_2_bus.body_yaw_rate_deg_s.SSM
                != static_cast<uint32_T>(SignStatusMatrix::NormalOperation)) ||
               (ElacComputer_U.in.bus_inputs.ir_2_bus.body_long_accel_g.SSM != static_cast<uint32_T>(SignStatusMatrix::
      NormalOperation)) || (ElacComputer_U.in.bus_inputs.ir_2_bus.body_lat_accel_g.SSM != static_cast<uint32_T>
                (SignStatusMatrix::NormalOperation)) || (ElacComputer_U.in.bus_inputs.ir_2_bus.body_normal_accel_g.SSM
                != static_cast<uint32_T>(SignStatusMatrix::NormalOperation)) ||
               (ElacComputer_U.in.bus_inputs.ir_2_bus.pitch_att_rate_deg_s.SSM != static_cast<uint32_T>(SignStatusMatrix::
      NormalOperation)) || (ElacComputer_U.in.bus_inputs.ir_2_bus.roll_att_rate_deg_s.SSM != static_cast<uint32_T>
                (SignStatusMatrix::NormalOperation)) || ElacComputer_P.Constant_Value_ad);
    rtb_OR7 = ((ElacComputer_U.in.bus_inputs.ir_3_bus.pitch_angle_deg.SSM != static_cast<uint32_T>(SignStatusMatrix::
      NormalOperation)) || (ElacComputer_U.in.bus_inputs.ir_3_bus.roll_angle_deg.SSM != static_cast<uint32_T>
                (SignStatusMatrix::NormalOperation)) || (ElacComputer_U.in.bus_inputs.ir_3_bus.body_yaw_rate_deg_s.SSM
                != static_cast<uint32_T>(SignStatusMatrix::NormalOperation)) ||
               (ElacComputer_U.in.bus_inputs.ir_3_bus.body_long_accel_g.SSM != static_cast<uint32_T>(SignStatusMatrix::
      NormalOperation)) || (ElacComputer_U.in.bus_inputs.ir_3_bus.body_lat_accel_g.SSM != static_cast<uint32_T>
                (SignStatusMatrix::NormalOperation)) || (ElacComputer_U.in.bus_inputs.ir_3_bus.body_normal_accel_g.SSM
                != static_cast<uint32_T>(SignStatusMatrix::NormalOperation)) ||
               (ElacComputer_U.in.bus_inputs.ir_3_bus.pitch_att_rate_deg_s.SSM != static_cast<uint32_T>(SignStatusMatrix::
      NormalOperation)) || (ElacComputer_U.in.bus_inputs.ir_3_bus.roll_att_rate_deg_s.SSM != static_cast<uint32_T>
                (SignStatusMatrix::NormalOperation)) || ElacComputer_P.Constant_Value_ad);
    rtb_tripleIrFault = (rtb_OR && rtb_OR6);
    rtb_ra1Invalid = (rtb_OR && rtb_OR7);
    rtb_doubleIrFault = (rtb_tripleIrFault || rtb_ra1Invalid || (rtb_OR6 && rtb_OR7));
    rtb_tripleIrFault = (rtb_tripleIrFault && rtb_OR7);
    rtb_AND1 = !rtb_OR4;
    rtb_AND2 = !rtb_OR3;
    if (rtb_OR1 && rtb_AND2 && rtb_AND1) {
      rtb_V_ias = (ElacComputer_U.in.bus_inputs.adr_2_bus.airspeed_computed_kn.Data +
                   ElacComputer_U.in.bus_inputs.adr_3_bus.airspeed_computed_kn.Data) / 2.0F;
      rtb_V_tas = (ElacComputer_U.in.bus_inputs.adr_2_bus.airspeed_true_kn.Data +
                   ElacComputer_U.in.bus_inputs.adr_3_bus.airspeed_true_kn.Data) / 2.0F;
      rtb_mach_h = (ElacComputer_U.in.bus_inputs.adr_2_bus.mach.Data + ElacComputer_U.in.bus_inputs.adr_3_bus.mach.Data)
        / 2.0F;
      rtb_alpha = (ElacComputer_U.in.bus_inputs.adr_2_bus.aoa_corrected_deg.Data +
                   ElacComputer_U.in.bus_inputs.adr_3_bus.aoa_corrected_deg.Data) / 2.0F;
    } else if ((!rtb_OR1) && rtb_OR3 && rtb_AND1) {
      rtb_V_ias = (ElacComputer_U.in.bus_inputs.adr_1_bus.airspeed_computed_kn.Data +
                   ElacComputer_U.in.bus_inputs.adr_3_bus.airspeed_computed_kn.Data) / 2.0F;
      rtb_V_tas = (ElacComputer_U.in.bus_inputs.adr_1_bus.airspeed_true_kn.Data +
                   ElacComputer_U.in.bus_inputs.adr_3_bus.airspeed_true_kn.Data) / 2.0F;
      rtb_mach_h = (ElacComputer_U.in.bus_inputs.adr_1_bus.mach.Data + ElacComputer_U.in.bus_inputs.adr_3_bus.mach.Data)
        / 2.0F;
      rtb_alpha = (ElacComputer_U.in.bus_inputs.adr_1_bus.aoa_corrected_deg.Data +
                   ElacComputer_U.in.bus_inputs.adr_3_bus.aoa_corrected_deg.Data) / 2.0F;
    } else if (((!rtb_OR1) && rtb_AND2 && rtb_AND1) || ((!rtb_OR1) && rtb_AND2 && rtb_OR4)) {
      rtb_V_ias = (ElacComputer_U.in.bus_inputs.adr_1_bus.airspeed_computed_kn.Data +
                   ElacComputer_U.in.bus_inputs.adr_2_bus.airspeed_computed_kn.Data) / 2.0F;
      rtb_V_tas = (ElacComputer_U.in.bus_inputs.adr_1_bus.airspeed_true_kn.Data +
                   ElacComputer_U.in.bus_inputs.adr_2_bus.airspeed_true_kn.Data) / 2.0F;
      rtb_mach_h = (ElacComputer_U.in.bus_inputs.adr_1_bus.mach.Data + ElacComputer_U.in.bus_inputs.adr_2_bus.mach.Data)
        / 2.0F;
      rtb_alpha = (ElacComputer_U.in.bus_inputs.adr_1_bus.aoa_corrected_deg.Data +
                   ElacComputer_U.in.bus_inputs.adr_2_bus.aoa_corrected_deg.Data) / 2.0F;
    } else if ((!rtb_OR1) && rtb_OR3 && rtb_OR4) {
      rtb_V_ias = ElacComputer_U.in.bus_inputs.adr_1_bus.airspeed_computed_kn.Data;
      rtb_V_tas = ElacComputer_U.in.bus_inputs.adr_1_bus.airspeed_true_kn.Data;
      rtb_mach_h = ElacComputer_U.in.bus_inputs.adr_1_bus.mach.Data;
      rtb_alpha = ElacComputer_U.in.bus_inputs.adr_1_bus.aoa_corrected_deg.Data;
    } else if (rtb_OR1 && rtb_AND2 && rtb_OR4) {
      rtb_V_ias = ElacComputer_U.in.bus_inputs.adr_2_bus.airspeed_computed_kn.Data;
      rtb_V_tas = ElacComputer_U.in.bus_inputs.adr_2_bus.airspeed_true_kn.Data;
      rtb_mach_h = ElacComputer_U.in.bus_inputs.adr_2_bus.mach.Data;
      rtb_alpha = ElacComputer_U.in.bus_inputs.adr_2_bus.aoa_corrected_deg.Data;
    } else if (rtb_OR1 && rtb_OR3 && rtb_AND1) {
      rtb_V_ias = ElacComputer_U.in.bus_inputs.adr_3_bus.airspeed_computed_kn.Data;
      rtb_V_tas = ElacComputer_U.in.bus_inputs.adr_3_bus.airspeed_true_kn.Data;
      rtb_mach_h = ElacComputer_U.in.bus_inputs.adr_3_bus.mach.Data;
      rtb_alpha = ElacComputer_U.in.bus_inputs.adr_3_bus.aoa_corrected_deg.Data;
    } else {
      rtb_V_ias = 0.0F;
      rtb_V_tas = 0.0F;
      rtb_mach_h = 0.0F;
      rtb_alpha = 0.0F;
    }

    ElacComputer_LagFilter(static_cast<real_T>(rtb_alpha), ElacComputer_P.LagFilter_C1, ElacComputer_U.in.time.dt,
      &rtb_Y, &ElacComputer_DWork.sf_LagFilter_a);
    rtb_AND1 = !rtb_OR6;
    rtb_AND2 = !rtb_OR7;
    rtb_OR1 = (rtb_OR && rtb_AND2);
    if (rtb_OR1 && rtb_AND1) {
      rtb_alpha = (ElacComputer_U.in.bus_inputs.ir_2_bus.pitch_angle_deg.Data +
                   ElacComputer_U.in.bus_inputs.ir_3_bus.pitch_angle_deg.Data) / 2.0F;
      rtb_phi = (ElacComputer_U.in.bus_inputs.ir_2_bus.roll_angle_deg.Data +
                 ElacComputer_U.in.bus_inputs.ir_3_bus.roll_angle_deg.Data) / 2.0F;
      rtb_q = (ElacComputer_U.in.bus_inputs.ir_2_bus.body_pitch_rate_deg_s.Data +
               ElacComputer_U.in.bus_inputs.ir_3_bus.body_pitch_rate_deg_s.Data) / 2.0F;
      rtb_r = (ElacComputer_U.in.bus_inputs.ir_2_bus.body_yaw_rate_deg_s.Data +
               ElacComputer_U.in.bus_inputs.ir_3_bus.body_yaw_rate_deg_s.Data) / 2.0F;
      rtb_n_x = (ElacComputer_U.in.bus_inputs.ir_2_bus.body_long_accel_g.Data +
                 ElacComputer_U.in.bus_inputs.ir_3_bus.body_long_accel_g.Data) / 2.0F;
      rtb_n_y = (ElacComputer_U.in.bus_inputs.ir_2_bus.body_lat_accel_g.Data +
                 ElacComputer_U.in.bus_inputs.ir_3_bus.body_lat_accel_g.Data) / 2.0F;
      rtb_n_z = (ElacComputer_U.in.bus_inputs.ir_2_bus.body_normal_accel_g.Data +
                 ElacComputer_U.in.bus_inputs.ir_3_bus.body_normal_accel_g.Data) / 2.0F;
      rtb_theta_dot = (ElacComputer_U.in.bus_inputs.ir_2_bus.pitch_att_rate_deg_s.Data +
                       ElacComputer_U.in.bus_inputs.ir_3_bus.pitch_att_rate_deg_s.Data) / 2.0F;
      rtb_phi_dot = (ElacComputer_U.in.bus_inputs.ir_2_bus.roll_att_rate_deg_s.Data +
                     ElacComputer_U.in.bus_inputs.ir_3_bus.roll_att_rate_deg_s.Data) / 2.0F;
    } else {
      rtb_OR = !rtb_OR;
      rtb_OR7 = (rtb_OR && rtb_OR7);
      if (rtb_OR7 && rtb_AND1) {
        rtb_alpha = (ElacComputer_U.in.bus_inputs.ir_1_bus.pitch_angle_deg.Data +
                     ElacComputer_U.in.bus_inputs.ir_2_bus.pitch_angle_deg.Data) / 2.0F;
        rtb_phi = (ElacComputer_U.in.bus_inputs.ir_1_bus.roll_angle_deg.Data +
                   ElacComputer_U.in.bus_inputs.ir_2_bus.roll_angle_deg.Data) / 2.0F;
        rtb_q = (ElacComputer_U.in.bus_inputs.ir_1_bus.body_pitch_rate_deg_s.Data +
                 ElacComputer_U.in.bus_inputs.ir_2_bus.body_pitch_rate_deg_s.Data) / 2.0F;
        rtb_r = (ElacComputer_U.in.bus_inputs.ir_1_bus.body_yaw_rate_deg_s.Data +
                 ElacComputer_U.in.bus_inputs.ir_2_bus.body_yaw_rate_deg_s.Data) / 2.0F;
        rtb_n_x = (ElacComputer_U.in.bus_inputs.ir_1_bus.body_long_accel_g.Data +
                   ElacComputer_U.in.bus_inputs.ir_2_bus.body_long_accel_g.Data) / 2.0F;
        rtb_n_y = (ElacComputer_U.in.bus_inputs.ir_1_bus.body_lat_accel_g.Data +
                   ElacComputer_U.in.bus_inputs.ir_2_bus.body_lat_accel_g.Data) / 2.0F;
        rtb_n_z = (ElacComputer_U.in.bus_inputs.ir_1_bus.body_normal_accel_g.Data +
                   ElacComputer_U.in.bus_inputs.ir_2_bus.body_normal_accel_g.Data) / 2.0F;
        rtb_theta_dot = (ElacComputer_U.in.bus_inputs.ir_1_bus.pitch_att_rate_deg_s.Data +
                         ElacComputer_U.in.bus_inputs.ir_2_bus.pitch_att_rate_deg_s.Data) / 2.0F;
        rtb_phi_dot = (ElacComputer_U.in.bus_inputs.ir_1_bus.roll_att_rate_deg_s.Data +
                       ElacComputer_U.in.bus_inputs.ir_2_bus.roll_att_rate_deg_s.Data) / 2.0F;
      } else {
        rtb_AND2 = (rtb_OR && rtb_AND2);
        if ((rtb_AND2 && rtb_AND1) || (rtb_AND2 && rtb_OR6)) {
          rtb_alpha = (ElacComputer_U.in.bus_inputs.ir_1_bus.pitch_angle_deg.Data +
                       ElacComputer_U.in.bus_inputs.ir_3_bus.pitch_angle_deg.Data) / 2.0F;
          rtb_phi = (ElacComputer_U.in.bus_inputs.ir_1_bus.roll_angle_deg.Data +
                     ElacComputer_U.in.bus_inputs.ir_3_bus.roll_angle_deg.Data) / 2.0F;
          rtb_q = (ElacComputer_U.in.bus_inputs.ir_1_bus.body_pitch_rate_deg_s.Data +
                   ElacComputer_U.in.bus_inputs.ir_3_bus.body_pitch_rate_deg_s.Data) / 2.0F;
          rtb_r = (ElacComputer_U.in.bus_inputs.ir_1_bus.body_yaw_rate_deg_s.Data +
                   ElacComputer_U.in.bus_inputs.ir_3_bus.body_yaw_rate_deg_s.Data) / 2.0F;
          rtb_n_x = (ElacComputer_U.in.bus_inputs.ir_1_bus.body_long_accel_g.Data +
                     ElacComputer_U.in.bus_inputs.ir_3_bus.body_long_accel_g.Data) / 2.0F;
          rtb_n_y = (ElacComputer_U.in.bus_inputs.ir_1_bus.body_lat_accel_g.Data +
                     ElacComputer_U.in.bus_inputs.ir_3_bus.body_lat_accel_g.Data) / 2.0F;
          rtb_n_z = (ElacComputer_U.in.bus_inputs.ir_1_bus.body_normal_accel_g.Data +
                     ElacComputer_U.in.bus_inputs.ir_3_bus.body_normal_accel_g.Data) / 2.0F;
          rtb_theta_dot = (ElacComputer_U.in.bus_inputs.ir_1_bus.pitch_att_rate_deg_s.Data +
                           ElacComputer_U.in.bus_inputs.ir_3_bus.pitch_att_rate_deg_s.Data) / 2.0F;
          rtb_phi_dot = (ElacComputer_U.in.bus_inputs.ir_1_bus.roll_att_rate_deg_s.Data +
                         ElacComputer_U.in.bus_inputs.ir_3_bus.roll_att_rate_deg_s.Data) / 2.0F;
        } else if (rtb_OR7 && rtb_OR6) {
          rtb_alpha = ElacComputer_U.in.bus_inputs.ir_1_bus.pitch_angle_deg.Data;
          rtb_phi = ElacComputer_U.in.bus_inputs.ir_1_bus.roll_angle_deg.Data;
          rtb_q = ElacComputer_U.in.bus_inputs.ir_1_bus.body_pitch_rate_deg_s.Data;
          rtb_r = ElacComputer_U.in.bus_inputs.ir_1_bus.body_yaw_rate_deg_s.Data;
          rtb_n_x = ElacComputer_U.in.bus_inputs.ir_1_bus.body_long_accel_g.Data;
          rtb_n_y = ElacComputer_U.in.bus_inputs.ir_1_bus.body_lat_accel_g.Data;
          rtb_n_z = ElacComputer_U.in.bus_inputs.ir_1_bus.body_normal_accel_g.Data;
          rtb_theta_dot = ElacComputer_U.in.bus_inputs.ir_1_bus.pitch_att_rate_deg_s.Data;
          rtb_phi_dot = ElacComputer_U.in.bus_inputs.ir_1_bus.roll_att_rate_deg_s.Data;
        } else if (rtb_OR1 && rtb_OR6) {
          rtb_alpha = ElacComputer_U.in.bus_inputs.ir_3_bus.pitch_angle_deg.Data;
          rtb_phi = ElacComputer_U.in.bus_inputs.ir_3_bus.roll_angle_deg.Data;
          rtb_q = ElacComputer_U.in.bus_inputs.ir_3_bus.body_pitch_rate_deg_s.Data;
          rtb_r = ElacComputer_U.in.bus_inputs.ir_3_bus.body_yaw_rate_deg_s.Data;
          rtb_n_x = ElacComputer_U.in.bus_inputs.ir_3_bus.body_long_accel_g.Data;
          rtb_n_y = ElacComputer_U.in.bus_inputs.ir_3_bus.body_lat_accel_g.Data;
          rtb_n_z = ElacComputer_U.in.bus_inputs.ir_3_bus.body_normal_accel_g.Data;
          rtb_theta_dot = ElacComputer_U.in.bus_inputs.ir_3_bus.pitch_att_rate_deg_s.Data;
          rtb_phi_dot = ElacComputer_U.in.bus_inputs.ir_3_bus.roll_att_rate_deg_s.Data;
        } else if (rtb_ra1Invalid && rtb_AND1) {
          rtb_alpha = ElacComputer_U.in.bus_inputs.ir_2_bus.pitch_angle_deg.Data;
          rtb_phi = ElacComputer_U.in.bus_inputs.ir_2_bus.roll_angle_deg.Data;
          rtb_q = ElacComputer_U.in.bus_inputs.ir_2_bus.body_pitch_rate_deg_s.Data;
          rtb_r = ElacComputer_U.in.bus_inputs.ir_2_bus.body_yaw_rate_deg_s.Data;
          rtb_n_x = ElacComputer_U.in.bus_inputs.ir_2_bus.body_long_accel_g.Data;
          rtb_n_y = ElacComputer_U.in.bus_inputs.ir_2_bus.body_lat_accel_g.Data;
          rtb_n_z = ElacComputer_U.in.bus_inputs.ir_2_bus.body_normal_accel_g.Data;
          rtb_theta_dot = ElacComputer_U.in.bus_inputs.ir_2_bus.pitch_att_rate_deg_s.Data;
          rtb_phi_dot = ElacComputer_U.in.bus_inputs.ir_2_bus.roll_att_rate_deg_s.Data;
        } else {
          rtb_alpha = 0.0F;
          rtb_phi = 0.0F;
          rtb_q = 0.0F;
          rtb_r = 0.0F;
          rtb_n_x = 0.0F;
          rtb_n_y = 0.0F;
          rtb_n_z = 0.0F;
          rtb_theta_dot = 0.0F;
          rtb_phi_dot = 0.0F;
        }
      }
    }

    ElacComputer_B.laws.lateral_law_outputs.right_aileron_command_deg = rtb_n_x;
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_actual_position_word,
      ElacComputer_P.BitfromLabel_bit, &rtb_y_b);
    ElacComputer_MATLABFunction(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_actual_position_word, &rtb_NOT_k);
    rtb_AND1 = ((rtb_y_b != 0U) && rtb_NOT_k);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_2_bus.slat_flap_actual_position_word,
      ElacComputer_P.BitfromLabel1_bit, &rtb_y_b);
    ElacComputer_MATLABFunction(&ElacComputer_U.in.bus_inputs.sfcc_2_bus.slat_flap_actual_position_word, &rtb_y_i);
    rtb_AND2 = ((rtb_y_b != 0U) && rtb_y_i);
    ElacComputer_MATLABFunction_c(std::abs(ElacComputer_U.in.bus_inputs.ra_1_bus.radio_height_ft.Data -
      ElacComputer_U.in.bus_inputs.ra_2_bus.radio_height_ft.Data) > ElacComputer_P.CompareToConstant_const_ll,
      ElacComputer_U.in.time.dt, ElacComputer_P.ConfirmNode_isRisingEdge, ElacComputer_P.ConfirmNode_timeDelay,
      &rtb_AND_ai, &ElacComputer_DWork.sf_MATLABFunction_jz);
    rtb_DataTypeConversion_by = (rtb_doubleAdrFault && ElacComputer_P.Constant1_Value_b);
    rtb_OR6 = (rtb_tripleAdrFault || rtb_DataTypeConversion_by);
    ElacComputer_MATLABFunction_c((ElacComputer_U.in.bus_inputs.ra_1_bus.radio_height_ft.Data > 50.0F) &&
      (ElacComputer_U.in.bus_inputs.ra_1_bus.radio_height_ft.SSM == static_cast<uint32_T>(SignStatusMatrix::
      NormalOperation)) && (rtb_V_ias > 200.0F) && rtb_OR6, ElacComputer_U.in.time.dt,
      ElacComputer_P.ConfirmNode2_isRisingEdge, ElacComputer_P.ConfirmNode2_timeDelay, &rtb_y_i,
      &ElacComputer_DWork.sf_MATLABFunction_lf);
    ElacComputer_MATLABFunction_c((ElacComputer_U.in.bus_inputs.ra_2_bus.radio_height_ft.Data > 50.0F) &&
      (ElacComputer_U.in.bus_inputs.ra_2_bus.radio_height_ft.SSM == static_cast<uint32_T>(SignStatusMatrix::
      NormalOperation)) && (rtb_V_ias > 200.0F) && rtb_OR6, ElacComputer_U.in.time.dt,
      ElacComputer_P.ConfirmNode1_isRisingEdge, ElacComputer_P.ConfirmNode1_timeDelay, &rtb_NOT_k,
      &ElacComputer_DWork.sf_MATLABFunction_jl);
    ElacComputer_DWork.ra1CoherenceRejected = (rtb_y_i || ElacComputer_DWork.ra1CoherenceRejected);
    ElacComputer_DWork.ra2CoherenceRejected = (rtb_NOT_k || ElacComputer_DWork.ra2CoherenceRejected);
    rtb_ra1Invalid = ((ElacComputer_U.in.bus_inputs.ra_1_bus.radio_height_ft.SSM == static_cast<uint32_T>
                       (SignStatusMatrix::FailureWarning)) || ElacComputer_DWork.ra1CoherenceRejected);
    rtb_OR7 = ((ElacComputer_U.in.bus_inputs.ra_2_bus.radio_height_ft.SSM == static_cast<uint32_T>(SignStatusMatrix::
      FailureWarning)) || ElacComputer_DWork.ra2CoherenceRejected);
    if (!ElacComputer_DWork.configFullEventTime_not_empty) {
      ElacComputer_DWork.configFullEventTime = ElacComputer_U.in.time.simulation_time;
      ElacComputer_DWork.configFullEventTime_not_empty = true;
    }

    if ((!rtb_AND1) && (!rtb_AND2)) {
      ElacComputer_DWork.configFullEventTime = ElacComputer_U.in.time.simulation_time;
    }

    rtb_AND1 = !rtb_OR7;
    rtb_AND2 = !rtb_ra1Invalid;
    if (rtb_AND2 && rtb_AND1) {
      if (rtb_AND_ai) {
        if (ElacComputer_U.in.time.simulation_time > ElacComputer_DWork.configFullEventTime + 10.0) {
          rtb_raComputationValue = std::fmin(ElacComputer_U.in.bus_inputs.ra_1_bus.radio_height_ft.Data,
            ElacComputer_U.in.bus_inputs.ra_2_bus.radio_height_ft.Data);
        } else {
          rtb_raComputationValue = 250.0F;
        }
      } else {
        rtb_raComputationValue = (ElacComputer_U.in.bus_inputs.ra_1_bus.radio_height_ft.Data +
          ElacComputer_U.in.bus_inputs.ra_2_bus.radio_height_ft.Data) / 2.0F;
      }
    } else if ((rtb_ra1Invalid && rtb_AND1) || (rtb_AND2 && rtb_OR7)) {
      if ((rtb_V_ias > 180.0F) && rtb_OR6) {
        rtb_raComputationValue = 250.0F;
      } else if (rtb_OR7) {
        rtb_raComputationValue = ElacComputer_U.in.bus_inputs.ra_1_bus.radio_height_ft.Data;
      } else {
        rtb_raComputationValue = ElacComputer_U.in.bus_inputs.ra_2_bus.radio_height_ft.Data;
      }
    } else {
      rtb_raComputationValue = 250.0F;
    }

    rtb_AND2 = (rtb_ra1Invalid && rtb_OR7);
    rtb_AND1_h = ((rtb_raComputationValue < ElacComputer_P.CompareToConstant_const) && (!rtb_AND2));
    rtb_AND1 = (ElacComputer_U.in.discrete_inputs.lgciu_1_left_main_gear_pressed &&
                ElacComputer_U.in.discrete_inputs.lgciu_1_right_main_gear_pressed);
    rtb_OR6 = (ElacComputer_U.in.discrete_inputs.ground_spoilers_active_1 &&
               ElacComputer_U.in.discrete_inputs.ground_spoilers_active_2);
    rtb_ra1Invalid = ((rtb_AND1 && ElacComputer_U.in.discrete_inputs.lgciu_2_left_main_gear_pressed &&
                       ElacComputer_U.in.discrete_inputs.lgciu_2_right_main_gear_pressed) || ((rtb_AND1 ||
      (ElacComputer_U.in.discrete_inputs.lgciu_2_left_main_gear_pressed &&
       ElacComputer_U.in.discrete_inputs.lgciu_2_right_main_gear_pressed)) && rtb_AND1_h) || (rtb_AND1_h && rtb_OR6));
    if (ElacComputer_DWork.is_active_c30_ElacComputer == 0U) {
      ElacComputer_DWork.is_active_c30_ElacComputer = 1U;
      ElacComputer_DWork.is_c30_ElacComputer = ElacComputer_IN_Ground;
      ElacComputer_B.in_flight = 0.0;
    } else {
      switch (ElacComputer_DWork.is_c30_ElacComputer) {
       case ElacComputer_IN_Flight:
        if (rtb_ra1Invalid && (rtb_alpha < 2.5F)) {
          ElacComputer_DWork.on_ground_time = ElacComputer_U.in.time.simulation_time;
          ElacComputer_DWork.is_c30_ElacComputer = ElacComputer_IN_FlightToGroundTransition;
        } else {
          ElacComputer_B.in_flight = 1.0;
        }
        break;

       case ElacComputer_IN_FlightToGroundTransition:
        if (ElacComputer_U.in.time.simulation_time - ElacComputer_DWork.on_ground_time >= 5.0) {
          ElacComputer_DWork.is_c30_ElacComputer = ElacComputer_IN_Ground;
          ElacComputer_B.in_flight = 0.0;
        } else if ((!rtb_ra1Invalid) || (rtb_alpha >= 2.5F)) {
          ElacComputer_DWork.on_ground_time = 0.0;
          ElacComputer_DWork.is_c30_ElacComputer = ElacComputer_IN_Flight;
          ElacComputer_B.in_flight = 1.0;
        }
        break;

       default:
        if (((!rtb_ra1Invalid) && (rtb_alpha > 8.0F)) || (rtb_raComputationValue > 400.0F)) {
          ElacComputer_DWork.on_ground_time = 0.0;
          ElacComputer_DWork.is_c30_ElacComputer = ElacComputer_IN_Flight;
          ElacComputer_B.in_flight = 1.0;
        } else {
          ElacComputer_B.in_flight = 0.0;
        }
        break;
      }
    }

    ElacComputer_MATLABFunction_m(ElacComputer_U.in.analog_inputs.yellow_hyd_pressure_psi,
      ElacComputer_P.HysteresisNode2_highTrigger, ElacComputer_P.HysteresisNode2_lowTrigger, &rtb_y_i,
      &ElacComputer_DWork.sf_MATLABFunction_jg);
    ElacComputer_MATLABFunction_c((!ElacComputer_U.in.discrete_inputs.yellow_low_pressure) && rtb_y_i,
      ElacComputer_U.in.time.dt, ElacComputer_P.ConfirmNode_isRisingEdge_k, ElacComputer_P.ConfirmNode_timeDelay_n,
      &rtb_AND_ai, &ElacComputer_DWork.sf_MATLABFunction_cj);
    ElacComputer_MATLABFunction_m(ElacComputer_U.in.analog_inputs.blue_hyd_pressure_psi,
      ElacComputer_P.HysteresisNode1_highTrigger, ElacComputer_P.HysteresisNode1_lowTrigger, &rtb_y_i,
      &ElacComputer_DWork.sf_MATLABFunction_mi);
    ElacComputer_MATLABFunction_c((!ElacComputer_U.in.discrete_inputs.blue_low_pressure) && rtb_y_i,
      ElacComputer_U.in.time.dt, ElacComputer_P.ConfirmNode1_isRisingEdge_i, ElacComputer_P.ConfirmNode1_timeDelay_h,
      &rtb_NOT_k, &ElacComputer_DWork.sf_MATLABFunction_g2);
    ElacComputer_MATLABFunction_m(ElacComputer_U.in.analog_inputs.green_hyd_pressure_psi,
      ElacComputer_P.HysteresisNode3_highTrigger, ElacComputer_P.HysteresisNode3_lowTrigger, &rtb_y_i,
      &ElacComputer_DWork.sf_MATLABFunction_br);
    ElacComputer_MATLABFunction_c((!ElacComputer_U.in.discrete_inputs.green_low_pressure) && rtb_y_i,
      ElacComputer_U.in.time.dt, ElacComputer_P.ConfirmNode2_isRisingEdge_j, ElacComputer_P.ConfirmNode2_timeDelay_k,
      &rtb_y_i, &ElacComputer_DWork.sf_MATLABFunction_gfx);
    rtb_OR7 = rtb_NOT_k;
    rtb_OR = rtb_y_i;
    ElacComputer_MATLABFunction_g(ElacComputer_U.in.discrete_inputs.capt_priority_takeover_pressed,
      ElacComputer_P.PulseNode_isRisingEdge, &rtb_NOT_k, &ElacComputer_DWork.sf_MATLABFunction_g4);
    ElacComputer_MATLABFunction_g(ElacComputer_U.in.discrete_inputs.fo_priority_takeover_pressed,
      ElacComputer_P.PulseNode1_isRisingEdge, &rtb_y_i, &ElacComputer_DWork.sf_MATLABFunction_nu);
    if (rtb_NOT_k) {
      ElacComputer_DWork.pRightStickDisabled = true;
      ElacComputer_DWork.pLeftStickDisabled = false;
    } else if (rtb_y_i) {
      ElacComputer_DWork.pLeftStickDisabled = true;
      ElacComputer_DWork.pRightStickDisabled = false;
    }

    if (ElacComputer_DWork.pRightStickDisabled && ((!ElacComputer_U.in.discrete_inputs.capt_priority_takeover_pressed) &&
         (!ElacComputer_DWork.Delay1_DSTATE))) {
      ElacComputer_DWork.pRightStickDisabled = false;
    } else if (ElacComputer_DWork.pLeftStickDisabled) {
      ElacComputer_DWork.pLeftStickDisabled = (ElacComputer_U.in.discrete_inputs.fo_priority_takeover_pressed ||
        ElacComputer_DWork.Delay_DSTATE_cc);
    }

    ElacComputer_MATLABFunction_c(ElacComputer_DWork.pLeftStickDisabled &&
      (ElacComputer_U.in.discrete_inputs.fo_priority_takeover_pressed || ElacComputer_DWork.Delay_DSTATE_cc),
      ElacComputer_U.in.time.dt, ElacComputer_P.ConfirmNode1_isRisingEdge_k, ElacComputer_P.ConfirmNode1_timeDelay_a,
      &ElacComputer_DWork.Delay_DSTATE_cc, &ElacComputer_DWork.sf_MATLABFunction_j2);
    ElacComputer_MATLABFunction_c(ElacComputer_DWork.pRightStickDisabled &&
      (ElacComputer_U.in.discrete_inputs.capt_priority_takeover_pressed || ElacComputer_DWork.Delay1_DSTATE),
      ElacComputer_U.in.time.dt, ElacComputer_P.ConfirmNode_isRisingEdge_j, ElacComputer_P.ConfirmNode_timeDelay_a,
      &ElacComputer_DWork.Delay1_DSTATE, &ElacComputer_DWork.sf_MATLABFunction_g24);
    if (ElacComputer_DWork.pLeftStickDisabled) {
      rtb_DataTypeConversion8 = ElacComputer_P.Constant1_Value_p;
    } else {
      rtb_DataTypeConversion8 = ElacComputer_U.in.analog_inputs.capt_roll_stick_pos;
    }

    if (!ElacComputer_DWork.pRightStickDisabled) {
      rtb_handleIndex = ElacComputer_U.in.analog_inputs.fo_roll_stick_pos;
    } else {
      rtb_handleIndex = ElacComputer_P.Constant1_Value_p;
    }

    rtb_Switch_b = rtb_handleIndex + rtb_DataTypeConversion8;
    if (rtb_Switch_b > ElacComputer_P.Saturation1_UpperSat) {
      rtb_Switch_b = ElacComputer_P.Saturation1_UpperSat;
    } else if (rtb_Switch_b < ElacComputer_P.Saturation1_LowerSat) {
      rtb_Switch_b = ElacComputer_P.Saturation1_LowerSat;
    }

    if (ElacComputer_U.in.discrete_inputs.is_unit_1) {
      rtb_OR1 = ((!ElacComputer_U.in.discrete_inputs.l_elev_servo_failed) && rtb_OR7);
      rtb_OR3 = ((!ElacComputer_U.in.discrete_inputs.r_elev_servo_failed) && rtb_OR7);
    } else {
      rtb_OR1 = ((!ElacComputer_U.in.discrete_inputs.l_elev_servo_failed) && rtb_OR);
      rtb_OR3 = ((!ElacComputer_U.in.discrete_inputs.r_elev_servo_failed) && rtb_AND_ai);
    }

    rtb_thsAvail_tmp = !ElacComputer_U.in.discrete_inputs.ths_motor_fault;
    rtb_OR4 = (rtb_thsAvail_tmp && (rtb_AND_ai || rtb_OR));
    if (ElacComputer_U.in.discrete_inputs.is_unit_1) {
      rtb_AND1 = rtb_OR7;
    } else {
      rtb_AND1 = ((rtb_AND_ai && rtb_OR) || ((!rtb_OR7) && (rtb_OR || rtb_AND_ai)));
    }

    rtb_thsAvail_tmp = ((!ElacComputer_U.in.discrete_inputs.r_elev_servo_failed) &&
                        (!ElacComputer_U.in.discrete_inputs.l_elev_servo_failed) && rtb_thsAvail_tmp && rtb_AND1);
    rtb_isEngagedInRoll = !ElacComputer_U.in.discrete_inputs.is_unit_1;
    hasPriorityInPitch = (rtb_isEngagedInRoll || ElacComputer_U.in.discrete_inputs.opp_axis_pitch_failure);
    rtb_isEngagedInPitch = (rtb_thsAvail_tmp && hasPriorityInPitch);
    if (ElacComputer_U.in.discrete_inputs.is_unit_1) {
      leftAileronAvail = ((!ElacComputer_U.in.discrete_inputs.l_ail_servo_failed) && rtb_OR7);
      rightAileronAvail = ((!ElacComputer_U.in.discrete_inputs.r_ail_servo_failed) && rtb_OR);
    } else {
      leftAileronAvail = ((!ElacComputer_U.in.discrete_inputs.l_ail_servo_failed) && rtb_OR);
      rightAileronAvail = ((!ElacComputer_U.in.discrete_inputs.r_ail_servo_failed) && rtb_OR7);
    }

    canEngageInRoll = (leftAileronAvail || rightAileronAvail);
    hasPriorityInRoll = (ElacComputer_U.in.discrete_inputs.is_unit_1 ||
                         (ElacComputer_U.in.discrete_inputs.opp_left_aileron_lost &&
                          ElacComputer_U.in.discrete_inputs.opp_right_aileron_lost));
    rtb_AND1 = !hasPriorityInRoll;
    if (rtb_isEngagedInRoll && rtb_AND1 && (ElacComputer_U.in.bus_inputs.elac_opp_bus.aileron_command_deg.SSM ==
         static_cast<uint32_T>(SignStatusMatrix::NormalOperation))) {
      ElacComputer_B.logic.left_aileron_crosscommand_active = (ElacComputer_U.in.discrete_inputs.opp_left_aileron_lost &&
        leftAileronAvail);
      ElacComputer_B.logic.right_aileron_crosscommand_active = (ElacComputer_U.in.discrete_inputs.opp_right_aileron_lost
        && rightAileronAvail);
    } else {
      ElacComputer_B.logic.left_aileron_crosscommand_active = false;
      ElacComputer_B.logic.right_aileron_crosscommand_active = false;
    }

    rtb_isEngagedInRoll = (canEngageInRoll && hasPriorityInRoll);
    ElacComputer_B.logic.is_yellow_hydraulic_power_avail = rtb_AND_ai;
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sec_1_bus.discrete_status_word_1,
      ElacComputer_P.BitfromLabel_bit_c, &rtb_y_b);
    rtb_AND2_p = (rtb_y_b != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sec_1_bus.discrete_status_word_1,
      ElacComputer_P.BitfromLabel1_bit_j, &rtb_y_b);
    rtb_AND1_h = (rtb_y_b != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sec_2_bus.discrete_status_word_1,
      ElacComputer_P.BitfromLabel2_bit, &rtb_y_b);
    rtb_AND4 = (rtb_y_b != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.fcdc_1_bus.efcs_status_word_3,
      ElacComputer_P.BitfromLabel3_bit, &rtb_y_b);
    rtb_AND_ai = (rtb_y_b != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.fcdc_2_bus.efcs_status_word_3,
      ElacComputer_P.BitfromLabel5_bit, &rtb_y_b);
    rtb_AND_ai = (rtb_AND_ai || (rtb_y_b != 0U));
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.fcdc_1_bus.efcs_status_word_3,
      ElacComputer_P.BitfromLabel4_bit, &rtb_y_b);
    rtb_NOT_k = (rtb_y_b != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.fcdc_2_bus.efcs_status_word_3,
      ElacComputer_P.BitfromLabel6_bit, &rtb_y_b);
    rtb_NOT_k = (rtb_NOT_k || (rtb_y_b != 0U));
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.fcdc_1_bus.efcs_status_word_1,
      ElacComputer_P.BitfromLabel7_bit, &rtb_y_b);
    rtb_y_i = (rtb_y_b == 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.fcdc_2_bus.efcs_status_word_1,
      ElacComputer_P.BitfromLabel8_bit, &rtb_y_b);
    if ((ElacComputer_U.in.discrete_inputs.fac_1_yaw_control_lost &&
         ElacComputer_U.in.discrete_inputs.fac_2_yaw_control_lost) ||
        ((ElacComputer_U.in.bus_inputs.sec_1_bus.discrete_status_word_1.SSM != static_cast<uint32_T>(SignStatusMatrix::
           NormalOperation)) && (ElacComputer_U.in.bus_inputs.sec_2_bus.discrete_status_word_1.SSM !=
          static_cast<uint32_T>(SignStatusMatrix::NormalOperation)) && ((!rtb_y_i) && (rtb_y_b != 0U))) || ((!rtb_AND2_p)
         && (!rtb_AND1_h) && (!rtb_AND4) && (!rtb_AND_ai) && (!rtb_NOT_k))) {
      rtb_lateralLawCapability = lateral_efcs_law::DirectLaw;
    } else {
      rtb_lateralLawCapability = lateral_efcs_law::NormalLaw;
    }

    ElacComputer_MATLABFunction(&ElacComputer_U.in.bus_inputs.sec_1_bus.discrete_status_word_2, &rtb_AND2_p);
    ElacComputer_MATLABFunction(&ElacComputer_U.in.bus_inputs.sec_2_bus.discrete_status_word_2, &rtb_y_i);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sec_1_bus.discrete_status_word_2,
      ElacComputer_P.BitfromLabel4_bit_d, &rtb_y_b);
    rtb_NOT_k = (rtb_y_b != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sec_2_bus.discrete_status_word_2,
      ElacComputer_P.BitfromLabel5_bit_e, &rtb_y_b);
    rtb_AND2_p = (((!rtb_AND2_p) && (!rtb_y_i)) || (rtb_NOT_k && (rtb_y_b != 0U)));
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sec_1_bus.discrete_status_word_2,
      ElacComputer_P.BitfromLabel6_bit_k, &rtb_y_b);
    rtb_NOT_k = (rtb_y_b != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sec_2_bus.discrete_status_word_2,
      ElacComputer_P.BitfromLabel7_bit_h, &rtb_y_b);
    rtb_AND_ai = (rtb_y_b != 0U);
    rtb_NOT_k = (rtb_NOT_k || (rtb_y_b != 0U));
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_actual_position_word,
      ElacComputer_P.BitfromLabel_bit_a, &rtb_y_b);
    ElacComputer_MATLABFunction(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_actual_position_word, &rtb_AND_ai);
    rtb_AND_ai = ((rtb_y_b != 0U) && rtb_AND_ai);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_2_bus.slat_flap_actual_position_word,
      ElacComputer_P.BitfromLabel1_bit_jr, &rtb_y_b);
    ElacComputer_MATLABFunction(&ElacComputer_U.in.bus_inputs.sfcc_2_bus.slat_flap_actual_position_word, &rtb_y_i);
    rtb_AND3_b = ((rtb_y_b != 0U) && rtb_y_i);
    ElacComputer_MATLABFunction_c(ElacComputer_U.in.sim_data.slew_on, ElacComputer_U.in.time.dt,
      ElacComputer_P.ConfirmNode_isRisingEdge_o, ElacComputer_P.ConfirmNode_timeDelay_d, &rtb_y_i,
      &ElacComputer_DWork.sf_MATLABFunction_nb);
    rtb_DataTypeConversion3_m = std::abs(static_cast<real_T>(rtb_phi));
    rtb_AND4 = !rtb_ra1Invalid;
    rtb_DataTypeConversion_cz = !ElacComputer_P.Constant_Value_ad;
    rtb_AND1_h = ((!rtb_y_i) && rtb_AND4 && (((!rtb_tripleAdrFault) && ((rtb_mach_h > 0.91) || (rtb_Y < -10.0) || (rtb_Y
      > 40.0) || (rtb_V_ias > 440.0F) || (rtb_V_ias < 60.0F))) || ((!rtb_tripleIrFault) && ((!rtb_doubleIrFault) ||
      rtb_DataTypeConversion_cz) && ((rtb_DataTypeConversion3_m > 125.0) || ((rtb_alpha > 50.0F) || (rtb_alpha < -30.0F))))));
    ElacComputer_DWork.abnormalConditionWasActive = (rtb_AND1_h || (rtb_AND4 &&
      ElacComputer_DWork.abnormalConditionWasActive));
    alternate2Condition_tmp = ((!rtb_OR1) || (!rtb_OR3));
    rtb_y_i = (rtb_DataTypeConversion_by || rtb_tripleAdrFault || ElacComputer_DWork.abnormalConditionWasActive ||
               ((!leftAileronAvail) && (!rightAileronAvail) && alternate2Condition_tmp));
    alternate1Condition_tmp = !ElacComputer_P.Constant1_Value_b;
    rtb_DataTypeConversion_by = ((rtb_doubleIrFault && rtb_DataTypeConversion_cz) || (rtb_DataTypeConversion_by &&
      alternate1Condition_tmp) || (rtb_doubleAdrFault && alternate1Condition_tmp && alternate1Condition_tmp) ||
      alternate2Condition_tmp);
    if (rtb_tripleIrFault || ((rtb_y_i || rtb_DataTypeConversion_by || rtb_AND2 || (rtb_lateralLawCapability ==
           lateral_efcs_law::DirectLaw)) && ((ElacComputer_B.in_flight != 0.0) && ((rtb_NOT_k && (!rtb_AND2_p)) ||
           ((rtb_AND_ai || rtb_AND3_b) && rtb_AND2_p))))) {
      rtb_pitchLawCapability = pitch_efcs_law::DirectLaw;
    } else if (rtb_y_i) {
      rtb_pitchLawCapability = pitch_efcs_law::AlternateLaw2;
    } else if (rtb_DataTypeConversion_by) {
      rtb_pitchLawCapability = pitch_efcs_law::AlternateLaw1;
    } else {
      rtb_pitchLawCapability = pitch_efcs_law::NormalLaw;
    }

    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.elac_opp_bus.discrete_status_word_2,
      ElacComputer_P.BitfromLabel_bit_h, &rtb_y_o);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.elac_opp_bus.discrete_status_word_2,
      ElacComputer_P.BitfromLabel1_bit_e, &rtb_Switch18);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.elac_opp_bus.discrete_status_word_2,
      ElacComputer_P.BitfromLabel2_bit_k, &rtb_y_e);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.elac_opp_bus.discrete_status_word_2,
      ElacComputer_P.BitfromLabel3_bit_m, &rtb_y_b);
    if ((rtb_y_e != 0U) && (rtb_y_b == 0U)) {
      rtb_oppElacRollCapability = lateral_efcs_law::NormalLaw;
    } else if ((rtb_y_e == 0U) && (rtb_y_b != 0U)) {
      rtb_oppElacRollCapability = lateral_efcs_law::DirectLaw;
    } else {
      rtb_oppElacRollCapability = lateral_efcs_law::None;
    }

    if (hasPriorityInPitch && rtb_isEngagedInPitch) {
      priorityPitchPitchLawCap = rtb_pitchLawCapability;
      priorityPitchLateralLawCap = rtb_lateralLawCapability;
    } else if ((!hasPriorityInPitch) || (!rtb_isEngagedInPitch)) {
      if ((rtb_y_o != 0U) && (rtb_Switch18 == 0U)) {
        priorityPitchPitchLawCap = pitch_efcs_law::NormalLaw;
      } else if ((rtb_y_o == 0U) && (rtb_Switch18 != 0U)) {
        priorityPitchPitchLawCap = pitch_efcs_law::AlternateLaw1;
      } else if ((rtb_y_o != 0U) && (rtb_Switch18 != 0U)) {
        priorityPitchPitchLawCap = pitch_efcs_law::DirectLaw;
      } else {
        priorityPitchPitchLawCap = pitch_efcs_law::None;
      }

      priorityPitchLateralLawCap = rtb_oppElacRollCapability;
    } else {
      priorityPitchPitchLawCap = pitch_efcs_law::None;
      priorityPitchLateralLawCap = lateral_efcs_law::None;
    }

    if (hasPriorityInRoll && rtb_isEngagedInRoll) {
      rtb_oppElacRollCapability = rtb_lateralLawCapability;
    } else if ((!rtb_AND1) && rtb_isEngagedInRoll) {
      rtb_oppElacRollCapability = lateral_efcs_law::None;
    }

    if (rtb_isEngagedInRoll) {
      if ((rtb_oppElacRollCapability == lateral_efcs_law::NormalLaw) && (priorityPitchPitchLawCap == pitch_efcs_law::
           NormalLaw) && (priorityPitchLateralLawCap == lateral_efcs_law::NormalLaw)) {
        rtb_activeLateralLaw = lateral_efcs_law::NormalLaw;
      } else {
        rtb_activeLateralLaw = lateral_efcs_law::DirectLaw;
      }
    } else {
      rtb_activeLateralLaw = lateral_efcs_law::None;
    }

    if (rtb_isEngagedInPitch) {
      if ((rtb_oppElacRollCapability == lateral_efcs_law::NormalLaw) && (priorityPitchPitchLawCap == pitch_efcs_law::
           NormalLaw) && (priorityPitchLateralLawCap == lateral_efcs_law::NormalLaw)) {
        priorityPitchPitchLawCap = pitch_efcs_law::NormalLaw;
      } else if ((rtb_oppElacRollCapability != lateral_efcs_law::NormalLaw) && (priorityPitchPitchLawCap ==
                  pitch_efcs_law::NormalLaw)) {
        priorityPitchPitchLawCap = pitch_efcs_law::AlternateLaw1;
      } else if (priorityPitchPitchLawCap != pitch_efcs_law::NormalLaw) {
        priorityPitchPitchLawCap = rtb_pitchLawCapability;
      } else {
        priorityPitchPitchLawCap = pitch_efcs_law::DirectLaw;
      }
    } else {
      priorityPitchPitchLawCap = pitch_efcs_law::None;
    }

    if (!ElacComputer_DWork.pRightStickDisabled) {
      rtb_handleIndex = ElacComputer_U.in.analog_inputs.fo_pitch_stick_pos;
    } else {
      rtb_handleIndex = ElacComputer_P.Constant_Value_p;
    }

    if (ElacComputer_DWork.pLeftStickDisabled) {
      rtb_Y_b = ElacComputer_P.Constant_Value_p;
    } else {
      rtb_Y_b = ElacComputer_U.in.analog_inputs.capt_pitch_stick_pos;
    }

    rtb_eta_trim_limit_lo_d = rtb_handleIndex + rtb_Y_b;
    if (rtb_eta_trim_limit_lo_d > ElacComputer_P.Saturation_UpperSat_d) {
      rtb_eta_trim_limit_lo_d = ElacComputer_P.Saturation_UpperSat_d;
    } else if (rtb_eta_trim_limit_lo_d < ElacComputer_P.Saturation_LowerSat_h) {
      rtb_eta_trim_limit_lo_d = ElacComputer_P.Saturation_LowerSat_h;
    }

    ElacComputer_MATLABFunction_g(ElacComputer_B.in_flight != 0.0, ElacComputer_P.PulseNode_isRisingEdge_g, &rtb_y_i,
      &ElacComputer_DWork.sf_MATLABFunction_l0);
    rtb_AND_ai = ((ElacComputer_U.in.discrete_inputs.is_unit_1 && rtb_OR4 && rtb_thsAvail_tmp) ||
                  (ElacComputer_U.in.discrete_inputs.is_unit_2 && rtb_OR4 && rtb_thsAvail_tmp &&
                   ElacComputer_U.in.discrete_inputs.opp_axis_pitch_failure));
    rtb_Y_c = std::abs(ElacComputer_U.in.analog_inputs.ths_pos_deg);
    ElacComputer_DWork.Memory_PreviousInput = ElacComputer_P.Logic_table[((((!rtb_AND_ai) || (rtb_Y_c <=
      ElacComputer_P.CompareToConstant_const_m) || ElacComputer_U.in.discrete_inputs.ths_override_active) + (
      static_cast<uint32_T>(rtb_y_i) << 1)) << 1) + ElacComputer_DWork.Memory_PreviousInput];
    rtb_NOT_k = (rtb_AND_ai && ElacComputer_DWork.Memory_PreviousInput);
    rtb_AND_ai = ((rtb_isEngagedInPitch && (ElacComputer_B.in_flight != 0.0) && ((priorityPitchPitchLawCap !=
      ElacComputer_P.EnumeratedConstant_Value_i) && (!rtb_AND1_h))) || rtb_NOT_k);
    rtb_logic_crg14_total_sidestick_roll_command = rtb_Switch_b;
    if (!ElacComputer_DWork.eventTime_not_empty_a) {
      ElacComputer_DWork.eventTime_g = ElacComputer_U.in.time.simulation_time;
      ElacComputer_DWork.eventTime_not_empty_a = true;
    }

    if (rtb_ra1Invalid || (ElacComputer_DWork.eventTime_g == 0.0)) {
      ElacComputer_DWork.eventTime_g = ElacComputer_U.in.time.simulation_time;
    }

    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word,
      ElacComputer_P.BitfromLabel_bit_h2, &rtb_y_b);
    rtb_DataTypeConversion_by = (rtb_y_b != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word,
      ElacComputer_P.BitfromLabel1_bit_g, &rtb_y_b);
    rtb_y_i = (rtb_y_b != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word,
      ElacComputer_P.BitfromLabel2_bit_n, &rtb_y_b);
    rtb_AND2_p = (rtb_y_b != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word,
      ElacComputer_P.BitfromLabel3_bit_g, &rtb_y_b);
    rtb_AND3_b = (rtb_y_b != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word,
      ElacComputer_P.BitfromLabel4_bit_e, &rtb_y_b);
    rtb_DataTypeConversion_cz = (rtb_y_b != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word,
      ElacComputer_P.BitfromLabel5_bit_a, &rtb_y_b);
    ElacComputer_MATLABFunction_o(rtb_DataTypeConversion_by, rtb_y_i, rtb_AND2_p, rtb_AND3_b, rtb_DataTypeConversion_cz,
      rtb_y_b != 0U, &rtb_handleIndex);
    ElacComputer_RateLimiter_n(look2_binlxpw(static_cast<real_T>(rtb_mach_h), rtb_handleIndex,
      ElacComputer_P.alphamax_bp01Data, ElacComputer_P.alphamax_bp02Data, ElacComputer_P.alphamax_tableData,
      ElacComputer_P.alphamax_maxIndex, 4U), ElacComputer_P.RateLimiterGenericVariableTs_up,
      ElacComputer_P.RateLimiterGenericVariableTs_lo, ElacComputer_U.in.time.dt, ElacComputer_P.reset_Value, &rtb_Y_b,
      &ElacComputer_DWork.sf_RateLimiter_n);
    ElacComputer_RateLimiter_n(look2_binlxpw(static_cast<real_T>(rtb_mach_h), rtb_handleIndex,
      ElacComputer_P.alphaprotection_bp01Data, ElacComputer_P.alphaprotection_bp02Data,
      ElacComputer_P.alphaprotection_tableData, ElacComputer_P.alphaprotection_maxIndex, 4U),
      ElacComputer_P.RateLimiterGenericVariableTs1_up, ElacComputer_P.RateLimiterGenericVariableTs1_lo,
      ElacComputer_U.in.time.dt, ElacComputer_P.reset_Value_j, &rtb_Y_c, &ElacComputer_DWork.sf_RateLimiter_m);
    if (ElacComputer_U.in.time.simulation_time - ElacComputer_DWork.eventTime_g <=
        ElacComputer_P.CompareToConstant_const_l) {
      rtb_DataTypeConversion8_g = rtb_Y_b;
    } else {
      rtb_DataTypeConversion8_g = rtb_Y_c;
    }

    ElacComputer_GetIASforMach4(static_cast<real_T>(rtb_mach_h), ElacComputer_P.Constant6_Value_b, static_cast<real_T>
      (rtb_V_ias), &rtb_Switch_b);
    rtb_Switch_b = std::fmin(ElacComputer_P.Constant5_Value_k, rtb_Switch_b);
    ElacComputer_GetIASforMach4(static_cast<real_T>(rtb_mach_h), ElacComputer_P.Constant8_Value_h, static_cast<real_T>
      (rtb_V_ias), &rtb_DataTypeConversion8);
    ElacComputer_B.logic.ths_ground_setting_active = rtb_NOT_k;
    if (ElacComputer_DWork.is_active_c28_ElacComputer == 0U) {
      ElacComputer_DWork.is_active_c28_ElacComputer = 1U;
      ElacComputer_DWork.is_c28_ElacComputer = ElacComputer_IN_Landed;
      rtb_ap_special_disc = 0;
    } else {
      switch (ElacComputer_DWork.is_c28_ElacComputer) {
       case ElacComputer_IN_Flying:
        if (rtb_raComputationValue < 100.0F) {
          ElacComputer_DWork.is_c28_ElacComputer = ElacComputer_IN_Landing100ft;
          rtb_ap_special_disc = 1;
        } else if (rtb_ra1Invalid) {
          ElacComputer_DWork.is_c28_ElacComputer = ElacComputer_IN_Landed;
          rtb_ap_special_disc = 0;
        } else {
          rtb_ap_special_disc = 0;
        }
        break;

       case ElacComputer_IN_Landed:
        if (!rtb_ra1Invalid) {
          ElacComputer_DWork.is_c28_ElacComputer = ElacComputer_IN_Takeoff100ft;
          rtb_ap_special_disc = 0;
        } else {
          rtb_ap_special_disc = 0;
        }
        break;

       case ElacComputer_IN_Landing100ft:
        if (rtb_raComputationValue > 100.0F) {
          ElacComputer_DWork.is_c28_ElacComputer = ElacComputer_IN_Flying;
          rtb_ap_special_disc = 0;
        } else if (rtb_ra1Invalid) {
          ElacComputer_DWork.is_c28_ElacComputer = ElacComputer_IN_Landed;
          rtb_ap_special_disc = 0;
        } else {
          rtb_ap_special_disc = 1;
        }
        break;

       default:
        if (rtb_ra1Invalid) {
          ElacComputer_DWork.is_c28_ElacComputer = ElacComputer_IN_Landed;
          rtb_ap_special_disc = 0;
        } else if (rtb_raComputationValue > 100.0F) {
          ElacComputer_DWork.is_c28_ElacComputer = ElacComputer_IN_Flying;
          rtb_ap_special_disc = 0;
        } else {
          rtb_ap_special_disc = 0;
        }
        break;
      }
    }

    rtb_Y_c = rtb_alpha - std::cos(ElacComputer_P.Gain1_Gain * rtb_phi) * rtb_Y;
    if (!ElacComputer_DWork.eventTime_not_empty) {
      ElacComputer_DWork.eventTime = ElacComputer_U.in.time.simulation_time;
      ElacComputer_DWork.eventTime_not_empty = true;
    }

    rtb_handleIndex = static_cast<real_T>(rtb_V_ias) / rtb_mach_h;
    if ((rtb_V_ias <= std::fmin(365.0, (look1_binlxpw(rtb_Y_c, ElacComputer_P.uDLookupTable_bp01Data,
            ElacComputer_P.uDLookupTable_tableData, 3U) + 0.01) * rtb_handleIndex)) || ((priorityPitchPitchLawCap !=
          pitch_efcs_law::NormalLaw) && (rtb_activeLateralLaw != lateral_efcs_law::NormalLaw)) ||
        (ElacComputer_DWork.eventTime == 0.0)) {
      ElacComputer_DWork.eventTime = ElacComputer_U.in.time.simulation_time;
    }

    rtb_NOT_k = ((priorityPitchPitchLawCap == pitch_efcs_law::NormalLaw) || (rtb_activeLateralLaw == lateral_efcs_law::
      NormalLaw));
    if (ElacComputer_U.in.discrete_inputs.ap_1_disengaged && ElacComputer_U.in.discrete_inputs.ap_2_disengaged &&
        (rtb_V_ias > std::fmin(look1_binlxpw(rtb_Y_c, ElacComputer_P.uDLookupTable1_bp01Data,
           ElacComputer_P.uDLookupTable1_tableData, 3U), rtb_handleIndex * look1_binlxpw(rtb_Y_c,
           ElacComputer_P.uDLookupTable2_bp01Data, ElacComputer_P.uDLookupTable2_tableData, 3U)))) {
      ElacComputer_DWork.sProtActive = (rtb_NOT_k || ElacComputer_DWork.sProtActive);
    }

    rtb_DataTypeConversion_by = (ElacComputer_U.in.discrete_inputs.ap_1_disengaged &&
      ElacComputer_U.in.discrete_inputs.ap_2_disengaged);
    ElacComputer_DWork.sProtActive = ((rtb_V_ias >= rtb_Switch_b) && (rtb_DataTypeConversion_by && rtb_NOT_k &&
      ElacComputer_DWork.sProtActive));
    rtb_NOT_k = ((priorityPitchPitchLawCap == pitch_efcs_law::NormalLaw) || (rtb_activeLateralLaw == lateral_efcs_law::
      NormalLaw));
    if (!ElacComputer_DWork.resetEventTime_not_empty) {
      ElacComputer_DWork.resetEventTime = ElacComputer_U.in.time.simulation_time;
      ElacComputer_DWork.resetEventTime_not_empty = true;
    }

    if ((rtb_eta_trim_limit_lo_d >= -0.03125) || (rtb_Y >= rtb_Y_b) || (ElacComputer_DWork.resetEventTime == 0.0)) {
      ElacComputer_DWork.resetEventTime = ElacComputer_U.in.time.simulation_time;
    }

    ElacComputer_DWork.sProtActive_m = ((rtb_AND4 && rtb_NOT_k && rtb_DataTypeConversion_by && (rtb_Y >
      rtb_DataTypeConversion8_g) && (ElacComputer_U.in.time.monotonic_time > 10.0)) || ElacComputer_DWork.sProtActive_m);
    ElacComputer_DWork.sProtActive_m = ((ElacComputer_U.in.time.simulation_time - ElacComputer_DWork.resetEventTime <=
      0.5) && (rtb_eta_trim_limit_lo_d >= -0.5) && ((rtb_raComputationValue >= 200.0F) || (rtb_eta_trim_limit_lo_d >=
      0.5) || (rtb_Y >= rtb_DataTypeConversion8_g - 2.0)) && rtb_AND4 && rtb_NOT_k && ElacComputer_DWork.sProtActive_m);
    rtb_NOT_k = ((rtb_AND4 && (((rtb_ap_special_disc != 0) && (rtb_Y > rtb_Y_b)) || (rtb_Y > rtb_DataTypeConversion8_g +
      0.25)) && ((priorityPitchPitchLawCap == pitch_efcs_law::NormalLaw) || (rtb_activeLateralLaw == lateral_efcs_law::
      NormalLaw))) || (ElacComputer_U.in.time.simulation_time - ElacComputer_DWork.eventTime > 3.0) ||
                 ElacComputer_DWork.sProtActive || ElacComputer_DWork.sProtActive_m);
    ElacComputer_B.logic.ths_active_commanded = rtb_AND_ai;
    ElacComputer_B.logic.protection_ap_disconnect = rtb_NOT_k;
    ElacComputer_B.logic.ap_authorised = ((std::abs(rtb_eta_trim_limit_lo_d) <= 0.5) && (std::abs
      (rtb_logic_crg14_total_sidestick_roll_command) <= 0.5) && ((std::abs
      (ElacComputer_U.in.analog_inputs.rudder_pedal_pos) <= 0.4) && ((rtb_alpha <= 25.0F) && (rtb_alpha >= -13.0F) &&
      (rtb_DataTypeConversion3_m <= 45.0) && ((!hasPriorityInPitch) || rtb_thsAvail_tmp) && (rtb_AND1 || canEngageInRoll)
      && (!rtb_NOT_k))));
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_actual_position_word,
      ElacComputer_P.BitfromLabel_bit_e, &rtb_y_b);
    rtb_AND1 = (rtb_y_b == 0U);
    ElacComputer_MATLABFunction(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_actual_position_word, &rtb_AND_ai);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_2_bus.slat_flap_actual_position_word,
      ElacComputer_P.BitfromLabel1_bit_d, &rtb_y_b);
    rtb_NOT_k = (rtb_y_b != 0U);
    ElacComputer_MATLABFunction(&ElacComputer_U.in.bus_inputs.sfcc_2_bus.slat_flap_actual_position_word, &rtb_NOT_k);
    rtb_AND1 = ((rtb_AND1 && rtb_AND_ai) || ((rtb_y_b == 0U) && rtb_NOT_k));
    ElacComputer_B.logic.on_ground = rtb_ra1Invalid;
    ElacComputer_B.logic.pitch_law_in_flight = (ElacComputer_B.in_flight != 0.0);
    ElacComputer_B.logic.tracking_mode_on = (ElacComputer_U.in.sim_data.slew_on || ElacComputer_U.in.sim_data.pause_on ||
      ElacComputer_U.in.sim_data.tracking_mode_on_override);
    ElacComputer_B.logic.lateral_law_capability = rtb_lateralLawCapability;
    ElacComputer_B.logic.active_lateral_law = rtb_activeLateralLaw;
    ElacComputer_B.logic.pitch_law_capability = rtb_pitchLawCapability;
    ElacComputer_B.logic.active_pitch_law = priorityPitchPitchLawCap;
    ElacComputer_B.logic.abnormal_condition_law_active = rtb_AND1_h;
    ElacComputer_B.logic.is_engaged_in_pitch = rtb_isEngagedInPitch;
    ElacComputer_B.logic.can_engage_in_pitch = rtb_thsAvail_tmp;
    ElacComputer_B.logic.has_priority_in_pitch = hasPriorityInPitch;
    ElacComputer_B.logic.left_elevator_avail = rtb_OR1;
    ElacComputer_B.logic.right_elevator_avail = rtb_OR3;
    ElacComputer_B.logic.ths_avail = rtb_OR4;
    ElacComputer_B.logic.is_engaged_in_roll = rtb_isEngagedInRoll;
    ElacComputer_B.logic.can_engage_in_roll = canEngageInRoll;
    ElacComputer_B.logic.has_priority_in_roll = hasPriorityInRoll;
    ElacComputer_B.logic.left_aileron_avail = leftAileronAvail;
    ElacComputer_B.logic.right_aileron_avail = rightAileronAvail;
    ElacComputer_B.logic.aileron_droop_active = (rtb_AND1 && ((leftAileronAvail && rightAileronAvail) ||
      ((!ElacComputer_U.in.discrete_inputs.opp_left_aileron_lost) && rightAileronAvail) || (leftAileronAvail &&
      (!ElacComputer_U.in.discrete_inputs.opp_right_aileron_lost))));
    ElacComputer_B.logic.aileron_antidroop_active = (rtb_OR6 && rtb_AND1 && (rtb_alpha < 2.5F) &&
      rtb_DataTypeConversion_by && (rtb_activeLateralLaw == lateral_efcs_law::NormalLaw));
    ElacComputer_B.logic.is_blue_hydraulic_power_avail = rtb_OR7;
    ElacComputer_B.logic.is_green_hydraulic_power_avail = rtb_OR;
    ElacComputer_B.logic.left_sidestick_disabled = ElacComputer_DWork.pLeftStickDisabled;
    ElacComputer_B.logic.right_sidestick_disabled = ElacComputer_DWork.pRightStickDisabled;
    ElacComputer_B.logic.left_sidestick_priority_locked = ElacComputer_DWork.Delay_DSTATE_cc;
    ElacComputer_B.logic.right_sidestick_priority_locked = ElacComputer_DWork.Delay1_DSTATE;
    ElacComputer_B.logic.total_sidestick_pitch_command = rtb_eta_trim_limit_lo_d;
    ElacComputer_B.logic.total_sidestick_roll_command = rtb_logic_crg14_total_sidestick_roll_command;
    ElacComputer_B.logic.high_alpha_prot_active = ElacComputer_DWork.sProtActive_m;
    ElacComputer_B.logic.alpha_prot_deg = rtb_DataTypeConversion8_g;
    ElacComputer_B.logic.alpha_max_deg = rtb_Y_b;
    ElacComputer_B.logic.high_speed_prot_active = ElacComputer_DWork.sProtActive;
    ElacComputer_B.logic.high_speed_prot_lo_thresh_kn = rtb_Switch_b;
    ElacComputer_B.logic.high_speed_prot_hi_thresh_kn = std::fmin(ElacComputer_P.Constant7_Value_g,
      rtb_DataTypeConversion8);
    ElacComputer_B.logic.double_adr_failure = rtb_doubleAdrFault;
    ElacComputer_B.logic.triple_adr_failure = rtb_tripleAdrFault;
    ElacComputer_B.logic.cas_or_mach_disagree = ElacComputer_P.Constant1_Value_b;
    ElacComputer_B.logic.alpha_disagree = ElacComputer_P.Constant1_Value_b;
    ElacComputer_B.logic.double_ir_failure = rtb_doubleIrFault;
    ElacComputer_B.logic.triple_ir_failure = rtb_tripleIrFault;
    ElacComputer_B.logic.ir_failure_not_self_detected = ElacComputer_P.Constant_Value_ad;
    ElacComputer_B.logic.adr_computation_data.V_ias_kn = rtb_V_ias;
    ElacComputer_B.logic.adr_computation_data.V_tas_kn = rtb_V_tas;
    ElacComputer_B.logic.adr_computation_data.mach = rtb_mach_h;
    ElacComputer_B.logic.adr_computation_data.alpha_deg = rtb_Y;
    ElacComputer_B.logic.ir_computation_data.theta_deg = rtb_alpha;
    ElacComputer_B.logic.ir_computation_data.phi_deg = rtb_phi;
    ElacComputer_B.logic.ir_computation_data.q_deg_s = rtb_q;
    ElacComputer_B.logic.ir_computation_data.r_deg_s = rtb_r;
    ElacComputer_B.logic.ir_computation_data.n_x_g = rtb_n_x;
    ElacComputer_B.logic.ir_computation_data.n_y_g = rtb_n_y;
    ElacComputer_B.logic.ir_computation_data.n_z_g = rtb_n_z;
    ElacComputer_B.logic.ir_computation_data.theta_dot_deg_s = rtb_theta_dot;
    ElacComputer_B.logic.ir_computation_data.phi_dot_deg_s = rtb_phi_dot;
    ElacComputer_B.logic.ra_computation_data_ft = rtb_raComputationValue;
    ElacComputer_B.logic.dual_ra_failure = rtb_AND2;
    if (ElacComputer_B.logic.aileron_droop_active) {
      rtb_handleIndex = ElacComputer_P.Constant2_Value;
    } else {
      rtb_handleIndex = ElacComputer_P.Constant1_Value;
    }

    ElacComputer_RateLimiter(rtb_handleIndex, ElacComputer_P.RateLimiterVariableTs2_up,
      ElacComputer_P.RateLimiterVariableTs2_lo, ElacComputer_U.in.time.dt,
      ElacComputer_P.RateLimiterVariableTs2_InitialCondition, &rtb_Y_b, &ElacComputer_DWork.sf_RateLimiter);
    if (ElacComputer_B.logic.aileron_antidroop_active) {
      rtb_handleIndex = ElacComputer_P.Constant4_Value_a;
    } else {
      rtb_handleIndex = ElacComputer_P.Constant3_Value;
    }

    ElacComputer_RateLimiter(rtb_handleIndex, ElacComputer_P.RateLimiterVariableTs3_up,
      ElacComputer_P.RateLimiterVariableTs3_lo, ElacComputer_U.in.time.dt,
      ElacComputer_P.RateLimiterVariableTs3_InitialCondition, &rtb_Y_c, &ElacComputer_DWork.sf_RateLimiter_b);
    rtb_Y_b += rtb_Y_c;
    rtb_tripleAdrFault = (ElacComputer_B.logic.tracking_mode_on || (static_cast<real_T>
      (ElacComputer_B.logic.active_lateral_law) != ElacComputer_P.CompareToConstant_const_m4));
    rtb_Y = ElacComputer_U.in.bus_inputs.fmgc_1_bus.delta_p_ail_cmd_deg.Data;
    rtb_Y_c = ElacComputer_U.in.bus_inputs.fmgc_1_bus.delta_r_cmd_deg.Data;
    rtb_doubleAdrFault = ((!ElacComputer_U.in.discrete_inputs.ap_1_disengaged) ||
                          (!ElacComputer_U.in.discrete_inputs.ap_2_disengaged));
    LawMDLOBJ2.step(&ElacComputer_U.in.time.dt, &ElacComputer_B.logic.ir_computation_data.theta_deg,
                    &ElacComputer_B.logic.ir_computation_data.phi_deg, &ElacComputer_B.logic.ir_computation_data.r_deg_s,
                    &ElacComputer_B.logic.ir_computation_data.phi_dot_deg_s,
                    &ElacComputer_B.logic.adr_computation_data.V_ias_kn,
                    &ElacComputer_B.logic.adr_computation_data.V_tas_kn, &ElacComputer_B.logic.ra_computation_data_ft,
                    &ElacComputer_B.logic.total_sidestick_roll_command,
                    &ElacComputer_U.in.analog_inputs.rudder_pedal_pos, &ElacComputer_B.logic.on_ground,
                    &rtb_tripleAdrFault, &ElacComputer_B.logic.high_alpha_prot_active,
                    &ElacComputer_B.logic.high_speed_prot_active, &rtb_Y, &rtb_Y_c, &rtb_doubleAdrFault, &rtb_xi_deg,
                    &rtb_zeta_deg);
    LawMDLOBJ1.step(&ElacComputer_U.in.time.dt, &ElacComputer_B.logic.total_sidestick_roll_command, &rtb_handleIndex,
                    &rtb_Y);
    switch (static_cast<int32_T>(ElacComputer_B.logic.active_lateral_law)) {
     case 0:
      rtb_handleIndex = rtb_xi_deg;
      break;

     case 1:
      break;

     default:
      rtb_handleIndex = ElacComputer_P.Constant_Value_c;
      break;
    }

    if (ElacComputer_B.logic.right_aileron_crosscommand_active) {
      rtb_Y_c = ElacComputer_U.in.bus_inputs.elac_opp_bus.aileron_command_deg.Data;
    } else {
      rtb_Y_c = rtb_handleIndex + rtb_Y_b;
    }

    if (rtb_Y_c > ElacComputer_P.Saturation2_UpperSat) {
      rtb_Y_c = ElacComputer_P.Saturation2_UpperSat;
    } else if (rtb_Y_c < ElacComputer_P.Saturation2_LowerSat) {
      rtb_Y_c = ElacComputer_P.Saturation2_LowerSat;
    }

    ElacComputer_RateLimiter_a(rtb_Y_c, ElacComputer_P.RateLimiterGenericVariableTs_up_b,
      ElacComputer_P.RateLimiterGenericVariableTs_lo_k, ElacComputer_U.in.time.dt,
      ElacComputer_U.in.analog_inputs.right_aileron_pos_deg, (!ElacComputer_B.logic.right_aileron_crosscommand_active) &&
      (!ElacComputer_B.logic.is_engaged_in_roll), &ElacComputer_B.laws.lateral_law_outputs.right_aileron_command_deg,
      &ElacComputer_DWork.sf_RateLimiter_a);
    if (ElacComputer_B.logic.left_aileron_crosscommand_active) {
      rtb_Y_c = ElacComputer_U.in.bus_inputs.elac_opp_bus.aileron_command_deg.Data;
    } else {
      rtb_Y_c = ElacComputer_P.Gain_Gain * rtb_handleIndex + rtb_Y_b;
    }

    if (rtb_Y_c > ElacComputer_P.Saturation1_UpperSat_g) {
      rtb_Y_c = ElacComputer_P.Saturation1_UpperSat_g;
    } else if (rtb_Y_c < ElacComputer_P.Saturation1_LowerSat_n) {
      rtb_Y_c = ElacComputer_P.Saturation1_LowerSat_n;
    }

    ElacComputer_RateLimiter_a(rtb_Y_c, ElacComputer_P.RateLimiterGenericVariableTs1_up_g,
      ElacComputer_P.RateLimiterGenericVariableTs1_lo_c, ElacComputer_U.in.time.dt,
      ElacComputer_U.in.analog_inputs.left_aileron_pos_deg, (!ElacComputer_B.logic.left_aileron_crosscommand_active) &&
      (!ElacComputer_B.logic.is_engaged_in_roll), &rtb_Y_b, &ElacComputer_DWork.sf_RateLimiter_p);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_actual_position_word,
      ElacComputer_P.BitfromLabel_bit_a2, &rtb_y_o);
    ElacComputer_MATLABFunction(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_actual_position_word, &rtb_AND2_p);
    rtb_tripleAdrFault = ((rtb_y_o != 0U) && rtb_AND2_p);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_2_bus.slat_flap_actual_position_word,
      ElacComputer_P.BitfromLabel1_bit_p, &rtb_y_o);
    ElacComputer_MATLABFunction(&ElacComputer_U.in.bus_inputs.sfcc_2_bus.slat_flap_actual_position_word, &rtb_AND_ai);
    if (rtb_tripleAdrFault || ((rtb_y_o != 0U) && rtb_AND_ai)) {
      rtb_Y_c = rtb_handleIndex;
    } else {
      rtb_eta_trim_limit_lo_d = std::abs(rtb_handleIndex) + ElacComputer_P.Bias_Bias;
      if (rtb_eta_trim_limit_lo_d > ElacComputer_P.Saturation_UpperSat) {
        rtb_eta_trim_limit_lo_d = ElacComputer_P.Saturation_UpperSat;
      } else if (rtb_eta_trim_limit_lo_d < ElacComputer_P.Saturation_LowerSat) {
        rtb_eta_trim_limit_lo_d = ElacComputer_P.Saturation_LowerSat;
      }

      if (rtb_handleIndex < 0.0) {
        rtb_Y_c = -1.0;
      } else {
        rtb_Y_c = (rtb_handleIndex > 0.0);
      }

      rtb_Y_c = rtb_eta_trim_limit_lo_d * rtb_Y_c * ElacComputer_P.Gain2_Gain;
    }

    ElacComputer_B.laws.lateral_law_outputs.roll_spoiler_command_deg = ElacComputer_P.Gain1_Gain_b * rtb_Y_c;
    switch (static_cast<int32_T>(ElacComputer_B.logic.active_lateral_law)) {
     case 0:
      ElacComputer_B.laws.lateral_law_outputs.yaw_damper_command_deg = rtb_zeta_deg;
      break;

     case 1:
      ElacComputer_B.laws.lateral_law_outputs.yaw_damper_command_deg = rtb_Y;
      break;

     default:
      ElacComputer_B.laws.lateral_law_outputs.yaw_damper_command_deg = ElacComputer_P.Constant_Value_c;
      break;
    }

    rtb_Y = ElacComputer_P.DiscreteDerivativeVariableTs_Gain * ElacComputer_B.logic.ir_computation_data.theta_dot_deg_s;
    ElacComputer_LagFilter((rtb_Y - ElacComputer_DWork.Delay_DSTATE) / ElacComputer_U.in.time.dt,
      ElacComputer_P.LagFilter_C1_e, ElacComputer_U.in.time.dt, &rtb_Y_c, &ElacComputer_DWork.sf_LagFilter);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word,
      ElacComputer_P.BitfromLabel_bit_p, &rtb_y_o);
    rtb_tripleAdrFault = (rtb_y_o != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word,
      ElacComputer_P.BitfromLabel1_bit_h, &rtb_y_o);
    rtb_doubleIrFault = (rtb_y_o != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word,
      ElacComputer_P.BitfromLabel2_bit_f, &rtb_y_o);
    rtb_tripleIrFault = (rtb_y_o != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word,
      ElacComputer_P.BitfromLabel3_bit_c, &rtb_y_o);
    rtb_AND1 = (rtb_y_o != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word,
      ElacComputer_P.BitfromLabel4_bit_n, &rtb_y_o);
    rtb_AND2 = (rtb_y_o != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word,
      ElacComputer_P.BitfromLabel5_bit_p, &rtb_y_o);
    ElacComputer_MATLABFunction_o(rtb_tripleAdrFault, rtb_doubleIrFault, rtb_tripleIrFault, rtb_AND1, rtb_AND2, rtb_y_o
      != 0U, &rtb_handleIndex);
    if ((ElacComputer_U.in.bus_inputs.sec_1_bus.thrust_lever_angle_1_deg.SSM == static_cast<uint32_T>(SignStatusMatrix::
          NormalOperation)) && (ElacComputer_U.in.bus_inputs.sec_1_bus.thrust_lever_angle_2_deg.SSM ==
         static_cast<uint32_T>(SignStatusMatrix::NormalOperation))) {
      rtb_V_ias = ElacComputer_U.in.bus_inputs.sec_1_bus.thrust_lever_angle_1_deg.Data;
      rtb_V_tas = ElacComputer_U.in.bus_inputs.sec_1_bus.thrust_lever_angle_2_deg.Data;
    } else if ((ElacComputer_U.in.bus_inputs.sec_2_bus.thrust_lever_angle_1_deg.SSM == static_cast<uint32_T>
                (SignStatusMatrix::NormalOperation)) &&
               (ElacComputer_U.in.bus_inputs.sec_2_bus.thrust_lever_angle_2_deg.SSM == static_cast<uint32_T>
                (SignStatusMatrix::NormalOperation))) {
      rtb_V_ias = ElacComputer_U.in.bus_inputs.sec_2_bus.thrust_lever_angle_1_deg.Data;
      rtb_V_tas = ElacComputer_U.in.bus_inputs.sec_2_bus.thrust_lever_angle_2_deg.Data;
    } else {
      rtb_V_ias = 0.0F;
      rtb_V_tas = 0.0F;
    }

    rtb_DataTypeConversion3_m = rtb_V_ias;
    rtb_DataTypeConversion8 = rtb_V_tas;
    rtb_eta_trim_limit_lo_d = ElacComputer_B.logic.pitch_law_in_flight;
    rtb_tripleAdrFault = (ElacComputer_B.logic.tracking_mode_on || (static_cast<real_T>
      (ElacComputer_B.logic.active_pitch_law) != ElacComputer_P.CompareToConstant_const_f));
    rtb_Switch_b = ElacComputer_U.in.bus_inputs.fmgc_1_bus.delta_q_cmd_deg.Data;
    LawMDLOBJ5.step(&ElacComputer_U.in.time.dt, &ElacComputer_B.logic.ir_computation_data.n_z_g,
                    &ElacComputer_B.logic.ir_computation_data.theta_deg,
                    &ElacComputer_B.logic.ir_computation_data.phi_deg,
                    &ElacComputer_B.logic.ir_computation_data.theta_dot_deg_s, &rtb_Y_c, (const_cast<real_T*>
      (&ElacComputer_RGND)), &ElacComputer_U.in.analog_inputs.ths_pos_deg,
                    &ElacComputer_B.logic.adr_computation_data.alpha_deg,
                    &ElacComputer_B.logic.adr_computation_data.V_ias_kn,
                    &ElacComputer_B.logic.adr_computation_data.V_tas_kn, &ElacComputer_B.logic.ra_computation_data_ft,
                    &rtb_handleIndex, (const_cast<real_T*>(&ElacComputer_RGND)), (const_cast<real_T*>(&ElacComputer_RGND)),
                    &rtb_DataTypeConversion3_m, &rtb_DataTypeConversion8,
                    &ElacComputer_U.in.sim_data.tailstrike_protection_on, (const_cast<real_T*>(&ElacComputer_RGND)),
                    &ElacComputer_B.logic.total_sidestick_pitch_command, &ElacComputer_B.logic.on_ground,
                    &rtb_eta_trim_limit_lo_d, &rtb_tripleAdrFault, &ElacComputer_B.logic.high_alpha_prot_active,
                    &ElacComputer_B.logic.high_speed_prot_active, &ElacComputer_B.logic.alpha_prot_deg,
                    &ElacComputer_B.logic.alpha_max_deg, &ElacComputer_B.logic.high_speed_prot_hi_thresh_kn,
                    &ElacComputer_B.logic.high_speed_prot_lo_thresh_kn, &rtb_Switch_b, &rtb_doubleAdrFault, &rtb_eta_deg,
                    &rtb_eta_trim_dot_deg_s, &rtb_eta_trim_limit_lo, &rtb_eta_trim_limit_up);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word,
      ElacComputer_P.BitfromLabel_bit_n, &rtb_y_o);
    rtb_tripleAdrFault = (rtb_y_o != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word,
      ElacComputer_P.BitfromLabel1_bit_h1, &rtb_y_o);
    rtb_doubleIrFault = (rtb_y_o != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word,
      ElacComputer_P.BitfromLabel2_bit_g, &rtb_y_o);
    rtb_tripleIrFault = (rtb_y_o != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word,
      ElacComputer_P.BitfromLabel3_bit_b, &rtb_y_o);
    rtb_AND1 = (rtb_y_o != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word,
      ElacComputer_P.BitfromLabel4_bit_i, &rtb_y_o);
    rtb_AND2 = (rtb_y_o != 0U);
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word,
      ElacComputer_P.BitfromLabel5_bit_l, &rtb_y_o);
    ElacComputer_MATLABFunction_o(rtb_tripleAdrFault, rtb_doubleIrFault, rtb_tripleIrFault, rtb_AND1, rtb_AND2, rtb_y_o
      != 0U, &rtb_Y_c);
    rtb_tripleAdrFault = (ElacComputer_B.logic.tracking_mode_on || ((static_cast<real_T>
      (ElacComputer_B.logic.active_pitch_law) != ElacComputer_P.CompareToConstant2_const) && (static_cast<real_T>
      (ElacComputer_B.logic.active_pitch_law) != ElacComputer_P.CompareToConstant3_const)));
    rtb_doubleIrFault = (ElacComputer_B.logic.active_pitch_law != ElacComputer_P.EnumeratedConstant_Value_b);
    LawMDLOBJ3.step(&ElacComputer_U.in.time.dt, &ElacComputer_B.logic.ir_computation_data.n_z_g,
                    &ElacComputer_B.logic.ir_computation_data.theta_deg,
                    &ElacComputer_B.logic.ir_computation_data.phi_deg,
                    &ElacComputer_B.logic.ir_computation_data.theta_dot_deg_s, (const_cast<real_T*>(&ElacComputer_RGND)),
                    &ElacComputer_U.in.analog_inputs.ths_pos_deg, &ElacComputer_B.logic.adr_computation_data.V_ias_kn,
                    &ElacComputer_B.logic.adr_computation_data.mach, &ElacComputer_B.logic.adr_computation_data.V_tas_kn,
                    &rtb_Y_c, (const_cast<real_T*>(&ElacComputer_RGND)), (const_cast<real_T*>(&ElacComputer_RGND)),
                    &ElacComputer_B.logic.total_sidestick_pitch_command, &rtb_eta_trim_limit_lo_d, &rtb_tripleAdrFault,
                    &rtb_doubleIrFault, &rtb_eta_deg_o, &rtb_eta_trim_dot_deg_s_a, &rtb_eta_trim_limit_lo_h,
                    &rtb_eta_trim_limit_up_d);
    LawMDLOBJ4.step(&ElacComputer_U.in.time.dt, &ElacComputer_B.logic.total_sidestick_pitch_command, &rtb_Y_c,
                    &rtb_DataTypeConversion8, &rtb_eta_trim_limit_lo_d, &rtb_Switch_b);
    switch (static_cast<int32_T>(ElacComputer_B.logic.active_pitch_law)) {
     case 0:
      rtb_Y_c = rtb_eta_deg;
      break;

     case 1:
     case 2:
      rtb_Y_c = rtb_eta_deg_o;
      break;

     case 3:
      break;

     default:
      rtb_Y_c = ElacComputer_P.Constant_Value_a;
      break;
    }

    switch (static_cast<int32_T>(ElacComputer_B.logic.active_pitch_law)) {
     case 0:
      rtb_Switch_b = rtb_eta_trim_limit_up;
      break;

     case 1:
     case 2:
      rtb_Switch_b = rtb_eta_trim_limit_up_d;
      break;

     case 3:
      break;

     default:
      rtb_Switch_b = ElacComputer_P.Constant2_Value_l;
      break;
    }

    if (ElacComputer_B.logic.ths_ground_setting_active) {
      rtb_DataTypeConversion8 = ElacComputer_P.Gain_Gain_l * ElacComputer_DWork.Delay_DSTATE_b;
      if (rtb_DataTypeConversion8 > ElacComputer_P.Saturation_UpperSat_g) {
        rtb_DataTypeConversion8 = ElacComputer_P.Saturation_UpperSat_g;
      } else if (rtb_DataTypeConversion8 < ElacComputer_P.Saturation_LowerSat_o) {
        rtb_DataTypeConversion8 = ElacComputer_P.Saturation_LowerSat_o;
      }
    } else if (ElacComputer_U.in.discrete_inputs.ths_override_active) {
      rtb_DataTypeConversion8 = ElacComputer_P.Constant_Value_n;
    } else {
      switch (static_cast<int32_T>(ElacComputer_B.logic.active_pitch_law)) {
       case 0:
        rtb_DataTypeConversion8 = rtb_eta_trim_dot_deg_s;
        break;

       case 1:
       case 2:
        rtb_DataTypeConversion8 = rtb_eta_trim_dot_deg_s_a;
        break;

       case 3:
        break;

       default:
        rtb_DataTypeConversion8 = ElacComputer_P.Constant_Value_a;
        break;
      }
    }

    rtb_DataTypeConversion8 = ElacComputer_P.DiscreteTimeIntegratorVariableTsLimit_Gain * rtb_DataTypeConversion8 *
      ElacComputer_U.in.time.dt;
    ElacComputer_DWork.icLoad = ((!ElacComputer_B.logic.ths_active_commanded) || ElacComputer_DWork.icLoad);
    if (ElacComputer_DWork.icLoad) {
      ElacComputer_DWork.Delay_DSTATE_c = ElacComputer_U.in.analog_inputs.ths_pos_deg - rtb_DataTypeConversion8;
    }

    ElacComputer_DWork.Delay_DSTATE_b = rtb_DataTypeConversion8 + ElacComputer_DWork.Delay_DSTATE_c;
    if (ElacComputer_DWork.Delay_DSTATE_b > rtb_Switch_b) {
      ElacComputer_DWork.Delay_DSTATE_b = rtb_Switch_b;
    } else {
      switch (static_cast<int32_T>(ElacComputer_B.logic.active_pitch_law)) {
       case 0:
        rtb_eta_trim_limit_lo_d = rtb_eta_trim_limit_lo;
        break;

       case 1:
       case 2:
        rtb_eta_trim_limit_lo_d = rtb_eta_trim_limit_lo_h;
        break;

       case 3:
        break;

       default:
        rtb_eta_trim_limit_lo_d = ElacComputer_P.Constant3_Value_h;
        break;
      }

      if (ElacComputer_DWork.Delay_DSTATE_b < rtb_eta_trim_limit_lo_d) {
        ElacComputer_DWork.Delay_DSTATE_b = rtb_eta_trim_limit_lo_d;
      }
    }

    ElacComputer_B.laws.lateral_law_outputs.left_aileron_command_deg = rtb_Y_b;
    ElacComputer_B.laws.pitch_law_outputs.elevator_command_deg = rtb_Y_c;
    ElacComputer_B.laws.pitch_law_outputs.ths_command_deg = ElacComputer_DWork.Delay_DSTATE_b;
    ElacComputer_B.laws.pitch_law_outputs.elevator_double_pressurization_active = ((look1_binlxpw
      (ElacComputer_B.logic.adr_computation_data.V_ias_kn, ElacComputer_P.uDLookupTable_bp01Data_h,
       ElacComputer_P.uDLookupTable_tableData_j, 6U) < std::abs(rtb_Y_c)) && ElacComputer_B.logic.is_engaged_in_pitch);
    ElacComputer_MATLABFunction(&ElacComputer_U.in.bus_inputs.elac_opp_bus.elevator_double_pressurization_command_deg,
      &rtb_AND_ai);
    rtb_NOT_k = ((!ElacComputer_B.logic.is_engaged_in_pitch) && rtb_AND_ai);
    if (rtb_NOT_k) {
      rtb_Y_c = ElacComputer_U.in.bus_inputs.elac_opp_bus.elevator_double_pressurization_command_deg.Data;
    } else {
      rtb_Y_c = ElacComputer_B.laws.pitch_law_outputs.elevator_command_deg;
    }

    if ((ElacComputer_B.logic.is_engaged_in_pitch || rtb_NOT_k) && ElacComputer_B.logic.left_elevator_avail) {
      ElacComputer_Y.out.analog_outputs.left_elev_pos_order_deg = rtb_Y_c;
    } else {
      ElacComputer_Y.out.analog_outputs.left_elev_pos_order_deg = ElacComputer_P.Constant_Value_b;
    }

    if ((rtb_NOT_k || ElacComputer_B.logic.is_engaged_in_pitch) && ElacComputer_B.logic.right_elevator_avail) {
      ElacComputer_Y.out.analog_outputs.right_elev_pos_order_deg = rtb_Y_c;
    } else {
      ElacComputer_Y.out.analog_outputs.right_elev_pos_order_deg = ElacComputer_P.Constant_Value_b;
    }

    if (ElacComputer_B.logic.ths_active_commanded && ElacComputer_B.logic.ths_avail) {
      ElacComputer_Y.out.analog_outputs.ths_pos_order = ElacComputer_B.laws.pitch_law_outputs.ths_command_deg;
    } else {
      ElacComputer_Y.out.analog_outputs.ths_pos_order = ElacComputer_P.Constant_Value_b;
    }

    rtb_AND1 = (ElacComputer_B.logic.is_engaged_in_roll || ElacComputer_B.logic.left_aileron_crosscommand_active);
    if (rtb_AND1 && ElacComputer_B.logic.left_aileron_avail) {
      ElacComputer_Y.out.analog_outputs.left_aileron_pos_order =
        ElacComputer_B.laws.lateral_law_outputs.left_aileron_command_deg;
    } else {
      ElacComputer_Y.out.analog_outputs.left_aileron_pos_order = ElacComputer_P.Constant_Value_b;
    }

    rtb_AND2 = (ElacComputer_B.logic.is_engaged_in_roll || ElacComputer_B.logic.right_aileron_crosscommand_active);
    if (rtb_AND2 && ElacComputer_B.logic.right_aileron_avail) {
      ElacComputer_Y.out.analog_outputs.right_aileron_pos_order =
        ElacComputer_B.laws.lateral_law_outputs.right_aileron_command_deg;
    } else {
      ElacComputer_Y.out.analog_outputs.right_aileron_pos_order = ElacComputer_P.Constant_Value_b;
    }

    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.sec_1_bus.discrete_status_word_1,
      ElacComputer_P.BitfromLabel_bit_es, &rtb_y_b);
    rtb_VectorConcatenate[17] = ((ElacComputer_B.logic.active_lateral_law == lateral_efcs_law::NormalLaw) ||
      ElacComputer_B.logic.abnormal_condition_law_active || ((rtb_y_b == 0U) &&
      (ElacComputer_U.in.bus_inputs.sec_1_bus.discrete_status_word_1.SSM == static_cast<uint32_T>(SignStatusMatrix::
      NormalOperation))));
    rtb_VectorConcatenate[18] = ((ElacComputer_B.logic.active_lateral_law == lateral_efcs_law::NormalLaw) ||
      ElacComputer_B.logic.abnormal_condition_law_active);
    rtb_y_e = static_cast<uint32_T>(ElacComputer_P.EnumeratedConstant1_Value);
    if (((!ElacComputer_B.logic.left_aileron_avail) || (!ElacComputer_B.logic.right_aileron_avail)) &&
        ElacComputer_B.logic.is_engaged_in_roll) {
      if (!ElacComputer_B.logic.left_aileron_avail) {
        rtb_V_ias = static_cast<real32_T>(ElacComputer_B.laws.lateral_law_outputs.left_aileron_command_deg);
      } else {
        rtb_V_ias = static_cast<real32_T>(ElacComputer_B.laws.lateral_law_outputs.right_aileron_command_deg);
      }

      ElacComputer_Y.out.bus_outputs.aileron_command_deg.SSM = static_cast<uint32_T>
        (ElacComputer_P.EnumeratedConstant1_Value);
    } else {
      rtb_V_ias = static_cast<real32_T>(ElacComputer_P.Constant5_Value);
      ElacComputer_Y.out.bus_outputs.aileron_command_deg.SSM = static_cast<uint32_T>
        (ElacComputer_P.EnumeratedConstant_Value);
    }

    if (ElacComputer_P.EnumeratedConstant2_Value == ElacComputer_B.logic.active_lateral_law) {
      rtb_V_tas = static_cast<real32_T>(ElacComputer_B.laws.lateral_law_outputs.yaw_damper_command_deg);
      rtb_y_b = static_cast<uint32_T>(ElacComputer_P.EnumeratedConstant1_Value);
    } else {
      rtb_V_tas = static_cast<real32_T>(ElacComputer_P.Constant7_Value);
      rtb_y_b = static_cast<uint32_T>(ElacComputer_P.EnumeratedConstant_Value);
    }

    if (ElacComputer_B.laws.pitch_law_outputs.elevator_double_pressurization_active) {
      rtb_mach_h = static_cast<real32_T>(ElacComputer_B.laws.pitch_law_outputs.elevator_command_deg);
    } else {
      rtb_mach_h = static_cast<real32_T>(ElacComputer_P.Constant8_Value);
    }

    ElacComputer_Y.out.bus_outputs.elevator_double_pressurization_command_deg.Data = rtb_mach_h;
    rtb_VectorConcatenate[0] = ElacComputer_U.in.discrete_inputs.l_ail_servo_failed;
    rtb_VectorConcatenate[9] = ElacComputer_B.logic.is_engaged_in_roll;
    rtb_VectorConcatenate[1] = ElacComputer_U.in.discrete_inputs.r_ail_servo_failed;
    rtb_VectorConcatenate[2] = ElacComputer_U.in.discrete_inputs.l_elev_servo_failed;
    rtb_VectorConcatenate[3] = ElacComputer_U.in.discrete_inputs.r_elev_servo_failed;
    rtb_VectorConcatenate[4] = ElacComputer_B.logic.left_aileron_avail;
    rtb_VectorConcatenate[5] = ElacComputer_B.logic.right_aileron_avail;
    rtb_VectorConcatenate[6] = ElacComputer_B.logic.left_elevator_avail;
    rtb_VectorConcatenate[7] = ElacComputer_B.logic.right_elevator_avail;
    rtb_VectorConcatenate[8] = ElacComputer_B.logic.is_engaged_in_pitch;
    rtb_VectorConcatenate[10] = !ElacComputer_B.logic.can_engage_in_pitch;
    rtb_VectorConcatenate[11] = !ElacComputer_B.logic.can_engage_in_roll;
    rtb_VectorConcatenate[12] = ((ElacComputer_B.logic.active_pitch_law == pitch_efcs_law::NormalLaw) ||
      (ElacComputer_B.logic.active_pitch_law == pitch_efcs_law::AlternateLaw2));
    rtb_VectorConcatenate[13] = ((ElacComputer_B.logic.active_pitch_law == pitch_efcs_law::AlternateLaw1) ||
      (ElacComputer_B.logic.active_pitch_law == pitch_efcs_law::AlternateLaw2));
    rtb_VectorConcatenate[14] = (ElacComputer_B.logic.active_pitch_law == pitch_efcs_law::DirectLaw);
    ElacComputer_LateralLawCaptoBits(ElacComputer_B.logic.active_lateral_law, &rtb_VectorConcatenate[15],
      &rtb_VectorConcatenate[16]);
    ElacComputer_MATLABFunction_cw(rtb_VectorConcatenate, &ElacComputer_Y.out.bus_outputs.discrete_status_word_1.Data);
    rtb_VectorConcatenate_a[9] = ElacComputer_B.logic.right_sidestick_priority_locked;
    rtb_VectorConcatenate_a[10] = ElacComputer_B.logic.aileron_droop_active;
    rtb_VectorConcatenate_a[12] = ElacComputer_B.logic.high_alpha_prot_active;
    rtb_VectorConcatenate_a[13] = ElacComputer_P.Constant10_Value;
    rtb_VectorConcatenate_a[14] = ElacComputer_P.Constant10_Value;
    rtb_VectorConcatenate_a[15] = ElacComputer_P.Constant10_Value;
    rtb_VectorConcatenate_a[16] = ElacComputer_P.Constant10_Value;
    rtb_VectorConcatenate_a[17] = ElacComputer_P.Constant10_Value;
    rtb_VectorConcatenate_a[18] = ElacComputer_P.Constant10_Value;
    rtb_VectorConcatenate_a[4] = ElacComputer_P.Constant9_Value;
    rtb_VectorConcatenate_a[5] = ElacComputer_P.Constant9_Value;
    rtb_VectorConcatenate_a[6] = ElacComputer_B.logic.left_sidestick_disabled;
    rtb_VectorConcatenate_a[7] = ElacComputer_B.logic.right_sidestick_disabled;
    rtb_VectorConcatenate_a[8] = ElacComputer_B.logic.left_sidestick_priority_locked;
    rtb_VectorConcatenate_a[0] = ((ElacComputer_B.logic.pitch_law_capability == pitch_efcs_law::NormalLaw) ||
      (ElacComputer_B.logic.pitch_law_capability == pitch_efcs_law::DirectLaw));
    rtb_VectorConcatenate_a[1] = ((ElacComputer_B.logic.pitch_law_capability == pitch_efcs_law::AlternateLaw1) ||
      (ElacComputer_B.logic.pitch_law_capability == pitch_efcs_law::AlternateLaw2) ||
      (ElacComputer_B.logic.pitch_law_capability == pitch_efcs_law::DirectLaw));
    ElacComputer_LateralLawCaptoBits(ElacComputer_B.logic.lateral_law_capability, &rtb_VectorConcatenate_a[2],
      &rtb_VectorConcatenate_a[3]);
    rtb_VectorConcatenate_a[11] = rtb_doubleAdrFault;
    ElacComputer_MATLABFunction_cw(rtb_VectorConcatenate_a, &rtb_mach_h);
    if (ElacComputer_U.in.discrete_inputs.l_ail_servo_failed) {
      ElacComputer_Y.out.bus_outputs.left_aileron_position_deg.SSM = static_cast<uint32_T>
        (ElacComputer_P.EnumeratedConstant_Value);
      ElacComputer_Y.out.bus_outputs.left_aileron_position_deg.Data = static_cast<real32_T>
        (ElacComputer_P.Constant_Value_j);
    } else {
      ElacComputer_Y.out.bus_outputs.left_aileron_position_deg.SSM = static_cast<uint32_T>
        (ElacComputer_P.EnumeratedConstant1_Value);
      ElacComputer_Y.out.bus_outputs.left_aileron_position_deg.Data = static_cast<real32_T>
        (ElacComputer_U.in.analog_inputs.left_aileron_pos_deg);
    }

    if (ElacComputer_U.in.discrete_inputs.r_ail_servo_failed) {
      ElacComputer_Y.out.bus_outputs.right_aileron_position_deg.SSM = static_cast<uint32_T>
        (ElacComputer_P.EnumeratedConstant_Value);
      ElacComputer_Y.out.bus_outputs.right_aileron_position_deg.Data = static_cast<real32_T>
        (ElacComputer_P.Constant1_Value_d);
    } else {
      ElacComputer_Y.out.bus_outputs.right_aileron_position_deg.SSM = static_cast<uint32_T>
        (ElacComputer_P.EnumeratedConstant1_Value);
      ElacComputer_Y.out.bus_outputs.right_aileron_position_deg.Data = static_cast<real32_T>
        (ElacComputer_U.in.analog_inputs.right_aileron_pos_deg);
    }

    if (ElacComputer_U.in.discrete_inputs.l_elev_servo_failed) {
      ElacComputer_Y.out.bus_outputs.left_elevator_position_deg.SSM = static_cast<uint32_T>
        (ElacComputer_P.EnumeratedConstant_Value);
      ElacComputer_Y.out.bus_outputs.left_elevator_position_deg.Data = static_cast<real32_T>
        (ElacComputer_P.Constant2_Value_b);
    } else {
      ElacComputer_Y.out.bus_outputs.left_elevator_position_deg.SSM = static_cast<uint32_T>
        (ElacComputer_P.EnumeratedConstant1_Value);
      ElacComputer_Y.out.bus_outputs.left_elevator_position_deg.Data = static_cast<real32_T>
        (ElacComputer_U.in.analog_inputs.left_elevator_pos_deg);
    }

    if (ElacComputer_U.in.discrete_inputs.r_elev_servo_failed) {
      ElacComputer_Y.out.bus_outputs.right_elevator_position_deg.SSM = static_cast<uint32_T>
        (ElacComputer_P.EnumeratedConstant_Value);
      ElacComputer_Y.out.bus_outputs.right_elevator_position_deg.Data = static_cast<real32_T>
        (ElacComputer_P.Constant3_Value_f);
    } else {
      ElacComputer_Y.out.bus_outputs.right_elevator_position_deg.SSM = static_cast<uint32_T>
        (ElacComputer_P.EnumeratedConstant1_Value);
      ElacComputer_Y.out.bus_outputs.right_elevator_position_deg.Data = static_cast<real32_T>
        (ElacComputer_U.in.analog_inputs.right_elevator_pos_deg);
    }

    if (ElacComputer_U.in.discrete_inputs.ths_motor_fault) {
      ElacComputer_Y.out.bus_outputs.ths_position_deg.SSM = static_cast<uint32_T>
        (ElacComputer_P.EnumeratedConstant_Value);
      ElacComputer_Y.out.bus_outputs.ths_position_deg.Data = static_cast<real32_T>(ElacComputer_P.Constant4_Value_i);
    } else {
      ElacComputer_Y.out.bus_outputs.ths_position_deg.SSM = static_cast<uint32_T>
        (ElacComputer_P.EnumeratedConstant1_Value);
      ElacComputer_Y.out.bus_outputs.ths_position_deg.Data = static_cast<real32_T>
        (ElacComputer_U.in.analog_inputs.ths_pos_deg);
    }

    ElacComputer_Y.out.bus_outputs.left_sidestick_pitch_command_deg.SSM = static_cast<uint32_T>
      (ElacComputer_P.EnumeratedConstant1_Value);
    ElacComputer_Y.out.bus_outputs.left_sidestick_pitch_command_deg.Data = ElacComputer_P.Gain_Gain_b *
      static_cast<real32_T>(ElacComputer_U.in.analog_inputs.capt_pitch_stick_pos);
    ElacComputer_Y.out.bus_outputs.right_sidestick_pitch_command_deg.SSM = static_cast<uint32_T>
      (ElacComputer_P.EnumeratedConstant1_Value);
    ElacComputer_Y.out.bus_outputs.right_sidestick_pitch_command_deg.Data = ElacComputer_P.Gain1_Gain_f * static_cast<
      real32_T>(ElacComputer_U.in.analog_inputs.fo_pitch_stick_pos);
    ElacComputer_Y.out.bus_outputs.left_sidestick_roll_command_deg.SSM = static_cast<uint32_T>
      (ElacComputer_P.EnumeratedConstant1_Value);
    ElacComputer_Y.out.bus_outputs.left_sidestick_roll_command_deg.Data = ElacComputer_P.Gain2_Gain_c *
      static_cast<real32_T>(ElacComputer_U.in.analog_inputs.capt_roll_stick_pos);
    ElacComputer_Y.out.bus_outputs.right_sidestick_roll_command_deg.SSM = static_cast<uint32_T>
      (ElacComputer_P.EnumeratedConstant1_Value);
    ElacComputer_Y.out.bus_outputs.right_sidestick_roll_command_deg.Data = ElacComputer_P.Gain3_Gain *
      static_cast<real32_T>(ElacComputer_U.in.analog_inputs.fo_roll_stick_pos);
    ElacComputer_Y.out.bus_outputs.rudder_pedal_position_deg.SSM = static_cast<uint32_T>
      (ElacComputer_P.EnumeratedConstant1_Value);
    ElacComputer_Y.out.bus_outputs.rudder_pedal_position_deg.Data = ElacComputer_P.Gain4_Gain * static_cast<real32_T>
      (ElacComputer_U.in.analog_inputs.rudder_pedal_pos);
    ElacComputer_Y.out.bus_outputs.aileron_command_deg.Data = rtb_V_ias;
    if (static_cast<real_T>(ElacComputer_B.logic.is_engaged_in_roll) > ElacComputer_P.Switch13_Threshold) {
      ElacComputer_Y.out.bus_outputs.roll_spoiler_command_deg.SSM = static_cast<uint32_T>
        (ElacComputer_P.EnumeratedConstant1_Value);
    } else {
      ElacComputer_Y.out.bus_outputs.roll_spoiler_command_deg.SSM = static_cast<uint32_T>
        (ElacComputer_P.EnumeratedConstant_Value);
    }

    if (static_cast<real_T>(ElacComputer_B.logic.is_engaged_in_roll) > ElacComputer_P.Switch12_Threshold) {
      ElacComputer_Y.out.bus_outputs.roll_spoiler_command_deg.Data = static_cast<real32_T>
        (ElacComputer_B.laws.lateral_law_outputs.roll_spoiler_command_deg);
    } else {
      ElacComputer_Y.out.bus_outputs.roll_spoiler_command_deg.Data = static_cast<real32_T>
        (ElacComputer_P.Constant6_Value);
    }

    ElacComputer_Y.out.bus_outputs.yaw_damper_command_deg.SSM = rtb_y_b;
    ElacComputer_Y.out.bus_outputs.yaw_damper_command_deg.Data = rtb_V_tas;
    if (ElacComputer_B.laws.pitch_law_outputs.elevator_double_pressurization_active) {
      ElacComputer_Y.out.bus_outputs.elevator_double_pressurization_command_deg.SSM = static_cast<uint32_T>
        (ElacComputer_P.EnumeratedConstant1_Value);
    } else {
      ElacComputer_Y.out.bus_outputs.elevator_double_pressurization_command_deg.SSM = static_cast<uint32_T>
        (ElacComputer_P.EnumeratedConstant_Value);
    }

    ElacComputer_Y.out.bus_outputs.discrete_status_word_1.SSM = static_cast<uint32_T>
      (ElacComputer_P.EnumeratedConstant1_Value);
    ElacComputer_Y.out.bus_outputs.discrete_status_word_2.SSM = static_cast<uint32_T>
      (ElacComputer_P.EnumeratedConstant1_Value);
    ElacComputer_Y.out.bus_outputs.discrete_status_word_2.Data = rtb_mach_h;
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.elac_opp_bus.discrete_status_word_1,
      ElacComputer_P.BitfromLabel_bit_p3, &rtb_y_b);
    rtb_NOT_k = (rtb_y_b != 0U);
    if (ElacComputer_U.in.discrete_inputs.is_unit_2) {
      rtb_y_b = ElacComputer_U.in.bus_inputs.sec_1_bus.discrete_status_word_1.SSM;
      rtb_mach_h = ElacComputer_U.in.bus_inputs.sec_1_bus.discrete_status_word_1.Data;
    } else {
      rtb_y_b = ElacComputer_U.in.bus_inputs.sec_2_bus.discrete_status_word_1.SSM;
      rtb_mach_h = ElacComputer_U.in.bus_inputs.sec_2_bus.discrete_status_word_1.Data;
    }

    rtb_BusConversion_InsertedFor_MATLABFunction_at_inport_0_BusCreator1_hi.SSM = rtb_y_b;
    rtb_BusConversion_InsertedFor_MATLABFunction_at_inport_0_BusCreator1_hi.Data = rtb_mach_h;
    ElacComputer_MATLABFunction_j(&rtb_BusConversion_InsertedFor_MATLABFunction_at_inport_0_BusCreator1_hi,
      ElacComputer_P.BitfromLabel2_bit_j, &rtb_y_e);
    rtb_NOT_k = (rtb_NOT_k || (rtb_y_e != 0U));
    ElacComputer_MATLABFunction_j(&ElacComputer_U.in.bus_inputs.elac_opp_bus.discrete_status_word_1,
      ElacComputer_P.BitfromLabel1_bit_i, &rtb_y_e);
    rtb_BusConversion_InsertedFor_MATLABFunction_at_inport_0_BusCreator1_hi.SSM = rtb_y_b;
    rtb_BusConversion_InsertedFor_MATLABFunction_at_inport_0_BusCreator1_hi.Data = rtb_mach_h;
    ElacComputer_MATLABFunction_j(&rtb_BusConversion_InsertedFor_MATLABFunction_at_inport_0_BusCreator1_hi,
      ElacComputer_P.BitfromLabel3_bit_mo, &rtb_y_b);
    ElacComputer_Y.out.discrete_outputs.pitch_axis_ok = ElacComputer_B.logic.can_engage_in_pitch;
    ElacComputer_Y.out.discrete_outputs.left_aileron_ok = ElacComputer_B.logic.left_aileron_avail;
    ElacComputer_Y.out.discrete_outputs.right_aileron_ok = ElacComputer_B.logic.right_aileron_avail;
    ElacComputer_Y.out.discrete_outputs.digital_output_validated = ElacComputer_P.Constant1_Value_e;
    ElacComputer_Y.out.discrete_outputs.ap_1_authorised = ElacComputer_B.logic.ap_authorised;
    ElacComputer_Y.out.discrete_outputs.ap_2_authorised = ElacComputer_B.logic.ap_authorised;
    ElacComputer_Y.out.discrete_outputs.left_aileron_active_mode = (rtb_AND1 && ElacComputer_B.logic.left_aileron_avail);
    ElacComputer_Y.out.discrete_outputs.right_aileron_active_mode = (rtb_AND2 &&
      ElacComputer_B.logic.right_aileron_avail);
    rtb_AND1 = !ElacComputer_B.laws.pitch_law_outputs.elevator_double_pressurization_active;
    ElacComputer_Y.out.discrete_outputs.left_elevator_damping_mode = (ElacComputer_B.logic.is_engaged_in_pitch &&
      ElacComputer_B.logic.left_elevator_avail && (rtb_AND1 || (!rtb_NOT_k)));
    ElacComputer_Y.out.discrete_outputs.right_elevator_damping_mode = (ElacComputer_B.logic.is_engaged_in_pitch &&
      ElacComputer_B.logic.right_elevator_avail && (((rtb_y_e == 0U) && (rtb_y_b == 0U)) || rtb_AND1));
    ElacComputer_Y.out.discrete_outputs.ths_active = (ElacComputer_B.logic.ths_active_commanded &&
      ElacComputer_B.logic.ths_avail);
    ElacComputer_B.dt = ElacComputer_U.in.time.dt;
    ElacComputer_B.ground_spoilers_active_2 = ElacComputer_U.in.discrete_inputs.ground_spoilers_active_2;
    ElacComputer_B.SSM = ElacComputer_U.in.bus_inputs.ir_1_bus.latitude_deg.SSM;
    ElacComputer_B.Data = ElacComputer_U.in.bus_inputs.ir_1_bus.latitude_deg.Data;
    ElacComputer_B.SSM_k = ElacComputer_U.in.bus_inputs.ir_1_bus.longitude_deg.SSM;
    ElacComputer_B.Data_f = ElacComputer_U.in.bus_inputs.ir_1_bus.longitude_deg.Data;
    ElacComputer_B.SSM_kx = ElacComputer_U.in.bus_inputs.ir_1_bus.ground_speed_kn.SSM;
    ElacComputer_B.Data_fw = ElacComputer_U.in.bus_inputs.ir_1_bus.ground_speed_kn.Data;
    ElacComputer_B.SSM_kxx = ElacComputer_U.in.bus_inputs.ir_1_bus.track_angle_true_deg.SSM;
    ElacComputer_B.Data_fwx = ElacComputer_U.in.bus_inputs.ir_1_bus.track_angle_true_deg.Data;
    ElacComputer_B.SSM_kxxt = ElacComputer_U.in.bus_inputs.ir_1_bus.heading_true_deg.SSM;
    ElacComputer_B.Data_fwxk = ElacComputer_U.in.bus_inputs.ir_1_bus.heading_true_deg.Data;
    ElacComputer_B.is_unit_1 = ElacComputer_U.in.discrete_inputs.is_unit_1;
    ElacComputer_B.SSM_kxxta = ElacComputer_U.in.bus_inputs.ir_1_bus.wind_speed_kn.SSM;
    ElacComputer_B.Data_fwxkf = ElacComputer_U.in.bus_inputs.ir_1_bus.wind_speed_kn.Data;
    ElacComputer_B.SSM_kxxtac = ElacComputer_U.in.bus_inputs.ir_1_bus.wind_direction_true_deg.SSM;
    ElacComputer_B.Data_fwxkft = ElacComputer_U.in.bus_inputs.ir_1_bus.wind_direction_true_deg.Data;
    ElacComputer_B.SSM_kxxtac0 = ElacComputer_U.in.bus_inputs.ir_1_bus.track_angle_magnetic_deg.SSM;
    ElacComputer_B.Data_fwxkftc = ElacComputer_U.in.bus_inputs.ir_1_bus.track_angle_magnetic_deg.Data;
    ElacComputer_B.SSM_kxxtac0z = ElacComputer_U.in.bus_inputs.ir_1_bus.heading_magnetic_deg.SSM;
    ElacComputer_B.Data_fwxkftc3 = ElacComputer_U.in.bus_inputs.ir_1_bus.heading_magnetic_deg.Data;
    ElacComputer_B.SSM_kxxtac0zt = ElacComputer_U.in.bus_inputs.ir_1_bus.drift_angle_deg.SSM;
    ElacComputer_B.Data_fwxkftc3e = ElacComputer_U.in.bus_inputs.ir_1_bus.drift_angle_deg.Data;
    ElacComputer_B.is_unit_2 = ElacComputer_U.in.discrete_inputs.is_unit_2;
    ElacComputer_B.SSM_kxxtac0ztg = ElacComputer_U.in.bus_inputs.ir_1_bus.flight_path_angle_deg.SSM;
    ElacComputer_B.Data_fwxkftc3ep = ElacComputer_U.in.bus_inputs.ir_1_bus.flight_path_angle_deg.Data;
    ElacComputer_B.SSM_kxxtac0ztgf = ElacComputer_U.in.bus_inputs.ir_1_bus.flight_path_accel_g.SSM;
    ElacComputer_B.Data_fwxkftc3epg = ElacComputer_U.in.bus_inputs.ir_1_bus.flight_path_accel_g.Data;
    ElacComputer_B.SSM_kxxtac0ztgf2 = ElacComputer_U.in.bus_inputs.ir_1_bus.pitch_angle_deg.SSM;
    ElacComputer_B.Data_fwxkftc3epgt = ElacComputer_U.in.bus_inputs.ir_1_bus.pitch_angle_deg.Data;
    ElacComputer_B.SSM_kxxtac0ztgf2u = ElacComputer_U.in.bus_inputs.ir_1_bus.roll_angle_deg.SSM;
    ElacComputer_B.Data_fwxkftc3epgtd = ElacComputer_U.in.bus_inputs.ir_1_bus.roll_angle_deg.Data;
    ElacComputer_B.SSM_kxxtac0ztgf2ux = ElacComputer_U.in.bus_inputs.ir_1_bus.body_pitch_rate_deg_s.SSM;
    ElacComputer_B.Data_fwxkftc3epgtdx = ElacComputer_U.in.bus_inputs.ir_1_bus.body_pitch_rate_deg_s.Data;
    ElacComputer_B.opp_axis_pitch_failure = ElacComputer_U.in.discrete_inputs.opp_axis_pitch_failure;
    ElacComputer_B.SSM_kxxtac0ztgf2uxn = ElacComputer_U.in.bus_inputs.ir_1_bus.body_roll_rate_deg_s.SSM;
    ElacComputer_B.Data_fwxkftc3epgtdxc = ElacComputer_U.in.bus_inputs.ir_1_bus.body_roll_rate_deg_s.Data;
    ElacComputer_B.SSM_ky = ElacComputer_U.in.bus_inputs.ir_1_bus.body_yaw_rate_deg_s.SSM;
    ElacComputer_B.Data_h = ElacComputer_U.in.bus_inputs.ir_1_bus.body_yaw_rate_deg_s.Data;
    ElacComputer_B.SSM_d = ElacComputer_U.in.bus_inputs.ir_1_bus.body_long_accel_g.SSM;
    ElacComputer_B.Data_e = ElacComputer_U.in.bus_inputs.ir_1_bus.body_long_accel_g.Data;
    ElacComputer_B.SSM_h = ElacComputer_U.in.bus_inputs.ir_1_bus.body_lat_accel_g.SSM;
    ElacComputer_B.Data_j = ElacComputer_U.in.bus_inputs.ir_1_bus.body_lat_accel_g.Data;
    ElacComputer_B.SSM_kb = ElacComputer_U.in.bus_inputs.ir_1_bus.body_normal_accel_g.SSM;
    ElacComputer_B.Data_d = ElacComputer_U.in.bus_inputs.ir_1_bus.body_normal_accel_g.Data;
    ElacComputer_B.ap_1_disengaged = ElacComputer_U.in.discrete_inputs.ap_1_disengaged;
    ElacComputer_B.SSM_p = ElacComputer_U.in.bus_inputs.ir_1_bus.track_angle_rate_deg_s.SSM;
    ElacComputer_B.Data_p = ElacComputer_U.in.bus_inputs.ir_1_bus.track_angle_rate_deg_s.Data;
    ElacComputer_B.SSM_di = ElacComputer_U.in.bus_inputs.ir_1_bus.pitch_att_rate_deg_s.SSM;
    ElacComputer_B.Data_i = ElacComputer_U.in.bus_inputs.ir_1_bus.pitch_att_rate_deg_s.Data;
    ElacComputer_B.SSM_j = ElacComputer_U.in.bus_inputs.ir_1_bus.roll_att_rate_deg_s.SSM;
    ElacComputer_B.Data_g = ElacComputer_U.in.bus_inputs.ir_1_bus.roll_att_rate_deg_s.Data;
    ElacComputer_B.SSM_i = ElacComputer_U.in.bus_inputs.ir_1_bus.inertial_alt_ft.SSM;
    ElacComputer_B.Data_a = ElacComputer_U.in.bus_inputs.ir_1_bus.inertial_alt_ft.Data;
    ElacComputer_B.SSM_g = ElacComputer_U.in.bus_inputs.ir_1_bus.along_track_horiz_acc_g.SSM;
    ElacComputer_B.Data_eb = ElacComputer_U.in.bus_inputs.ir_1_bus.along_track_horiz_acc_g.Data;
    ElacComputer_B.ap_2_disengaged = ElacComputer_U.in.discrete_inputs.ap_2_disengaged;
    ElacComputer_B.SSM_db = ElacComputer_U.in.bus_inputs.ir_1_bus.cross_track_horiz_acc_g.SSM;
    ElacComputer_B.Data_jo = ElacComputer_U.in.bus_inputs.ir_1_bus.cross_track_horiz_acc_g.Data;
    ElacComputer_B.SSM_n = ElacComputer_U.in.bus_inputs.ir_1_bus.vertical_accel_g.SSM;
    ElacComputer_B.Data_ex = ElacComputer_U.in.bus_inputs.ir_1_bus.vertical_accel_g.Data;
    ElacComputer_B.SSM_a = ElacComputer_U.in.bus_inputs.ir_1_bus.inertial_vertical_speed_ft_s.SSM;
    ElacComputer_B.Data_fd = ElacComputer_U.in.bus_inputs.ir_1_bus.inertial_vertical_speed_ft_s.Data;
    ElacComputer_B.SSM_ir = ElacComputer_U.in.bus_inputs.ir_1_bus.north_south_velocity_kn.SSM;
    ElacComputer_B.Data_ja = ElacComputer_U.in.bus_inputs.ir_1_bus.north_south_velocity_kn.Data;
    ElacComputer_B.SSM_hu = ElacComputer_U.in.bus_inputs.ir_1_bus.east_west_velocity_kn.SSM;
    ElacComputer_B.Data_k = ElacComputer_U.in.bus_inputs.ir_1_bus.east_west_velocity_kn.Data;
    ElacComputer_B.opp_left_aileron_lost = ElacComputer_U.in.discrete_inputs.opp_left_aileron_lost;
    ElacComputer_B.SSM_e = ElacComputer_U.in.bus_inputs.ir_2_bus.discrete_word_1.SSM;
    ElacComputer_B.Data_joy = ElacComputer_U.in.bus_inputs.ir_2_bus.discrete_word_1.Data;
    ElacComputer_B.SSM_gr = ElacComputer_U.in.bus_inputs.ir_2_bus.latitude_deg.SSM;
    ElacComputer_B.Data_h3 = ElacComputer_U.in.bus_inputs.ir_2_bus.latitude_deg.Data;
    ElacComputer_B.SSM_ev = ElacComputer_U.in.bus_inputs.ir_2_bus.longitude_deg.SSM;
    ElacComputer_B.Data_a0 = ElacComputer_U.in.bus_inputs.ir_2_bus.longitude_deg.Data;
    ElacComputer_B.SSM_l = ElacComputer_U.in.bus_inputs.ir_2_bus.ground_speed_kn.SSM;
    ElacComputer_B.Data_b = ElacComputer_U.in.bus_inputs.ir_2_bus.ground_speed_kn.Data;
    ElacComputer_B.SSM_ei = ElacComputer_U.in.bus_inputs.ir_2_bus.track_angle_true_deg.SSM;
    ElacComputer_B.Data_eq = ElacComputer_U.in.bus_inputs.ir_2_bus.track_angle_true_deg.Data;
    ElacComputer_B.opp_right_aileron_lost = ElacComputer_U.in.discrete_inputs.opp_right_aileron_lost;
    ElacComputer_B.SSM_an = ElacComputer_U.in.bus_inputs.ir_2_bus.heading_true_deg.SSM;
    ElacComputer_B.Data_iz = ElacComputer_U.in.bus_inputs.ir_2_bus.heading_true_deg.Data;
    ElacComputer_B.SSM_c = ElacComputer_U.in.bus_inputs.ir_2_bus.wind_speed_kn.SSM;
    ElacComputer_B.Data_j2 = ElacComputer_U.in.bus_inputs.ir_2_bus.wind_speed_kn.Data;
    ElacComputer_B.SSM_cb = ElacComputer_U.in.bus_inputs.ir_2_bus.wind_direction_true_deg.SSM;
    ElacComputer_B.Data_o = ElacComputer_U.in.bus_inputs.ir_2_bus.wind_direction_true_deg.Data;
    ElacComputer_B.SSM_lb = ElacComputer_U.in.bus_inputs.ir_2_bus.track_angle_magnetic_deg.SSM;
    ElacComputer_B.Data_m = ElacComputer_U.in.bus_inputs.ir_2_bus.track_angle_magnetic_deg.Data;
    ElacComputer_B.SSM_ia = ElacComputer_U.in.bus_inputs.ir_2_bus.heading_magnetic_deg.SSM;
    ElacComputer_B.Data_oq = ElacComputer_U.in.bus_inputs.ir_2_bus.heading_magnetic_deg.Data;
    ElacComputer_B.fac_1_yaw_control_lost = ElacComputer_U.in.discrete_inputs.fac_1_yaw_control_lost;
    ElacComputer_B.SSM_kyz = ElacComputer_U.in.bus_inputs.ir_2_bus.drift_angle_deg.SSM;
    ElacComputer_B.Data_fo = ElacComputer_U.in.bus_inputs.ir_2_bus.drift_angle_deg.Data;
    ElacComputer_B.SSM_as = ElacComputer_U.in.bus_inputs.ir_2_bus.flight_path_angle_deg.SSM;
    ElacComputer_B.Data_p1 = ElacComputer_U.in.bus_inputs.ir_2_bus.flight_path_angle_deg.Data;
    ElacComputer_B.SSM_is = ElacComputer_U.in.bus_inputs.ir_2_bus.flight_path_accel_g.SSM;
    ElacComputer_B.Data_p1y = ElacComputer_U.in.bus_inputs.ir_2_bus.flight_path_accel_g.Data;
    ElacComputer_B.SSM_ca = ElacComputer_U.in.bus_inputs.ir_2_bus.pitch_angle_deg.SSM;
    ElacComputer_B.Data_l = ElacComputer_U.in.bus_inputs.ir_2_bus.pitch_angle_deg.Data;
    ElacComputer_B.SSM_o = ElacComputer_U.in.bus_inputs.ir_2_bus.roll_angle_deg.SSM;
    ElacComputer_B.Data_kp = ElacComputer_U.in.bus_inputs.ir_2_bus.roll_angle_deg.Data;
    ElacComputer_B.lgciu_1_nose_gear_pressed = ElacComputer_U.in.discrete_inputs.lgciu_1_nose_gear_pressed;
    ElacComputer_B.SSM_ak = ElacComputer_U.in.bus_inputs.ir_2_bus.body_pitch_rate_deg_s.SSM;
    ElacComputer_B.Data_k0 = ElacComputer_U.in.bus_inputs.ir_2_bus.body_pitch_rate_deg_s.Data;
    ElacComputer_B.SSM_cbj = ElacComputer_U.in.bus_inputs.ir_2_bus.body_roll_rate_deg_s.SSM;
    ElacComputer_B.Data_pi = ElacComputer_U.in.bus_inputs.ir_2_bus.body_roll_rate_deg_s.Data;
    ElacComputer_B.SSM_cu = ElacComputer_U.in.bus_inputs.ir_2_bus.body_yaw_rate_deg_s.SSM;
    ElacComputer_B.Data_dm = ElacComputer_U.in.bus_inputs.ir_2_bus.body_yaw_rate_deg_s.Data;
    ElacComputer_B.SSM_nn = ElacComputer_U.in.bus_inputs.ir_2_bus.body_long_accel_g.SSM;
    ElacComputer_B.Data_f5 = ElacComputer_U.in.bus_inputs.ir_2_bus.body_long_accel_g.Data;
    ElacComputer_B.SSM_b = ElacComputer_U.in.bus_inputs.ir_2_bus.body_lat_accel_g.SSM;
    ElacComputer_B.Data_js = ElacComputer_U.in.bus_inputs.ir_2_bus.body_lat_accel_g.Data;
    ElacComputer_B.simulation_time = ElacComputer_U.in.time.simulation_time;
    ElacComputer_B.lgciu_2_nose_gear_pressed = ElacComputer_U.in.discrete_inputs.lgciu_2_nose_gear_pressed;
    ElacComputer_B.SSM_m = ElacComputer_U.in.bus_inputs.ir_2_bus.body_normal_accel_g.SSM;
    ElacComputer_B.Data_ee = ElacComputer_U.in.bus_inputs.ir_2_bus.body_normal_accel_g.Data;
    ElacComputer_B.SSM_f = ElacComputer_U.in.bus_inputs.ir_2_bus.track_angle_rate_deg_s.SSM;
    ElacComputer_B.Data_ig = ElacComputer_U.in.bus_inputs.ir_2_bus.track_angle_rate_deg_s.Data;
    ElacComputer_B.SSM_bp = ElacComputer_U.in.bus_inputs.ir_2_bus.pitch_att_rate_deg_s.SSM;
    ElacComputer_B.Data_mk = ElacComputer_U.in.bus_inputs.ir_2_bus.pitch_att_rate_deg_s.Data;
    ElacComputer_B.SSM_hb = ElacComputer_U.in.bus_inputs.ir_2_bus.roll_att_rate_deg_s.SSM;
    ElacComputer_B.Data_pu = ElacComputer_U.in.bus_inputs.ir_2_bus.roll_att_rate_deg_s.Data;
    ElacComputer_B.SSM_gz = ElacComputer_U.in.bus_inputs.ir_2_bus.inertial_alt_ft.SSM;
    ElacComputer_B.Data_ly = ElacComputer_U.in.bus_inputs.ir_2_bus.inertial_alt_ft.Data;
    ElacComputer_B.fac_2_yaw_control_lost = ElacComputer_U.in.discrete_inputs.fac_2_yaw_control_lost;
    ElacComputer_B.SSM_pv = ElacComputer_U.in.bus_inputs.ir_2_bus.along_track_horiz_acc_g.SSM;
    ElacComputer_B.Data_jq = ElacComputer_U.in.bus_inputs.ir_2_bus.along_track_horiz_acc_g.Data;
    ElacComputer_B.SSM_mf = ElacComputer_U.in.bus_inputs.ir_2_bus.cross_track_horiz_acc_g.SSM;
    ElacComputer_B.Data_o5 = ElacComputer_U.in.bus_inputs.ir_2_bus.cross_track_horiz_acc_g.Data;
    ElacComputer_B.SSM_m0 = ElacComputer_U.in.bus_inputs.ir_2_bus.vertical_accel_g.SSM;
    ElacComputer_B.Data_lyw = ElacComputer_U.in.bus_inputs.ir_2_bus.vertical_accel_g.Data;
    ElacComputer_B.SSM_kd = ElacComputer_U.in.bus_inputs.ir_2_bus.inertial_vertical_speed_ft_s.SSM;
    ElacComputer_B.Data_gq = ElacComputer_U.in.bus_inputs.ir_2_bus.inertial_vertical_speed_ft_s.Data;
    ElacComputer_B.SSM_pu = ElacComputer_U.in.bus_inputs.ir_2_bus.north_south_velocity_kn.SSM;
    ElacComputer_B.Data_n = ElacComputer_U.in.bus_inputs.ir_2_bus.north_south_velocity_kn.Data;
    ElacComputer_B.lgciu_1_right_main_gear_pressed = ElacComputer_U.in.discrete_inputs.lgciu_1_right_main_gear_pressed;
    ElacComputer_B.SSM_nv = ElacComputer_U.in.bus_inputs.ir_2_bus.east_west_velocity_kn.SSM;
    ElacComputer_B.Data_bq = ElacComputer_U.in.bus_inputs.ir_2_bus.east_west_velocity_kn.Data;
    ElacComputer_B.SSM_d5 = ElacComputer_U.in.bus_inputs.ir_3_bus.discrete_word_1.SSM;
    ElacComputer_B.Data_dmn = ElacComputer_U.in.bus_inputs.ir_3_bus.discrete_word_1.Data;
    ElacComputer_B.SSM_eo = ElacComputer_U.in.bus_inputs.ir_3_bus.latitude_deg.SSM;
    ElacComputer_B.Data_jn = ElacComputer_U.in.bus_inputs.ir_3_bus.latitude_deg.Data;
    ElacComputer_B.SSM_nd = ElacComputer_U.in.bus_inputs.ir_3_bus.longitude_deg.SSM;
    ElacComputer_B.Data_c = ElacComputer_U.in.bus_inputs.ir_3_bus.longitude_deg.Data;
    ElacComputer_B.SSM_bq = ElacComputer_U.in.bus_inputs.ir_3_bus.ground_speed_kn.SSM;
    ElacComputer_B.Data_lx = ElacComputer_U.in.bus_inputs.ir_3_bus.ground_speed_kn.Data;
    ElacComputer_B.lgciu_2_right_main_gear_pressed = ElacComputer_U.in.discrete_inputs.lgciu_2_right_main_gear_pressed;
    ElacComputer_B.SSM_hi = ElacComputer_U.in.bus_inputs.ir_3_bus.track_angle_true_deg.SSM;
    ElacComputer_B.Data_jb = ElacComputer_U.in.bus_inputs.ir_3_bus.track_angle_true_deg.Data;
    ElacComputer_B.SSM_mm = ElacComputer_U.in.bus_inputs.ir_3_bus.heading_true_deg.SSM;
    ElacComputer_B.Data_fn = ElacComputer_U.in.bus_inputs.ir_3_bus.heading_true_deg.Data;
    ElacComputer_B.SSM_kz = ElacComputer_U.in.bus_inputs.ir_3_bus.wind_speed_kn.SSM;
    ElacComputer_B.Data_od = ElacComputer_U.in.bus_inputs.ir_3_bus.wind_speed_kn.Data;
    ElacComputer_B.SSM_il = ElacComputer_U.in.bus_inputs.ir_3_bus.wind_direction_true_deg.SSM;
    ElacComputer_B.Data_ez = ElacComputer_U.in.bus_inputs.ir_3_bus.wind_direction_true_deg.Data;
    ElacComputer_B.SSM_i2 = ElacComputer_U.in.bus_inputs.ir_3_bus.track_angle_magnetic_deg.SSM;
    ElacComputer_B.Data_pw = ElacComputer_U.in.bus_inputs.ir_3_bus.track_angle_magnetic_deg.Data;
    ElacComputer_B.lgciu_1_left_main_gear_pressed = ElacComputer_U.in.discrete_inputs.lgciu_1_left_main_gear_pressed;
    ElacComputer_B.SSM_ah = ElacComputer_U.in.bus_inputs.ir_3_bus.heading_magnetic_deg.SSM;
    ElacComputer_B.Data_m2 = ElacComputer_U.in.bus_inputs.ir_3_bus.heading_magnetic_deg.Data;
    ElacComputer_B.SSM_en = ElacComputer_U.in.bus_inputs.ir_3_bus.drift_angle_deg.SSM;
    ElacComputer_B.Data_ek = ElacComputer_U.in.bus_inputs.ir_3_bus.drift_angle_deg.Data;
    ElacComputer_B.SSM_dq = ElacComputer_U.in.bus_inputs.ir_3_bus.flight_path_angle_deg.SSM;
    ElacComputer_B.Data_iy = ElacComputer_U.in.bus_inputs.ir_3_bus.flight_path_angle_deg.Data;
    ElacComputer_B.SSM_px = ElacComputer_U.in.bus_inputs.ir_3_bus.flight_path_accel_g.SSM;
    ElacComputer_B.Data_lk = ElacComputer_U.in.bus_inputs.ir_3_bus.flight_path_accel_g.Data;
    ElacComputer_B.SSM_lbo = ElacComputer_U.in.bus_inputs.ir_3_bus.pitch_angle_deg.SSM;
    ElacComputer_B.Data_ca = ElacComputer_U.in.bus_inputs.ir_3_bus.pitch_angle_deg.Data;
    ElacComputer_B.lgciu_2_left_main_gear_pressed = ElacComputer_U.in.discrete_inputs.lgciu_2_left_main_gear_pressed;
    ElacComputer_B.SSM_p5 = ElacComputer_U.in.bus_inputs.ir_3_bus.roll_angle_deg.SSM;
    ElacComputer_B.Data_pix = ElacComputer_U.in.bus_inputs.ir_3_bus.roll_angle_deg.Data;
    ElacComputer_B.SSM_mk = ElacComputer_U.in.bus_inputs.ir_3_bus.body_pitch_rate_deg_s.SSM;
    ElacComputer_B.Data_di = ElacComputer_U.in.bus_inputs.ir_3_bus.body_pitch_rate_deg_s.Data;
    ElacComputer_B.SSM_mu = ElacComputer_U.in.bus_inputs.ir_3_bus.body_roll_rate_deg_s.SSM;
    ElacComputer_B.Data_lz = ElacComputer_U.in.bus_inputs.ir_3_bus.body_roll_rate_deg_s.Data;
    ElacComputer_B.SSM_cbl = ElacComputer_U.in.bus_inputs.ir_3_bus.body_yaw_rate_deg_s.SSM;
    ElacComputer_B.Data_lu = ElacComputer_U.in.bus_inputs.ir_3_bus.body_yaw_rate_deg_s.Data;
    ElacComputer_B.SSM_gzd = ElacComputer_U.in.bus_inputs.ir_3_bus.body_long_accel_g.SSM;
    ElacComputer_B.Data_dc = ElacComputer_U.in.bus_inputs.ir_3_bus.body_long_accel_g.Data;
    ElacComputer_B.ths_motor_fault = ElacComputer_U.in.discrete_inputs.ths_motor_fault;
    ElacComputer_B.SSM_mo = ElacComputer_U.in.bus_inputs.ir_3_bus.body_lat_accel_g.SSM;
    ElacComputer_B.Data_gc = ElacComputer_U.in.bus_inputs.ir_3_bus.body_lat_accel_g.Data;
    ElacComputer_B.SSM_me = ElacComputer_U.in.bus_inputs.ir_3_bus.body_normal_accel_g.SSM;
    ElacComputer_B.Data_am = ElacComputer_U.in.bus_inputs.ir_3_bus.body_normal_accel_g.Data;
    ElacComputer_B.SSM_mj = ElacComputer_U.in.bus_inputs.ir_3_bus.track_angle_rate_deg_s.SSM;
    ElacComputer_B.Data_mo = ElacComputer_U.in.bus_inputs.ir_3_bus.track_angle_rate_deg_s.Data;
    ElacComputer_B.SSM_a5 = ElacComputer_U.in.bus_inputs.ir_3_bus.pitch_att_rate_deg_s.SSM;
    ElacComputer_B.Data_dg = ElacComputer_U.in.bus_inputs.ir_3_bus.pitch_att_rate_deg_s.Data;
    ElacComputer_B.SSM_bt = ElacComputer_U.in.bus_inputs.ir_3_bus.roll_att_rate_deg_s.SSM;
    ElacComputer_B.Data_e1 = ElacComputer_U.in.bus_inputs.ir_3_bus.roll_att_rate_deg_s.Data;
    ElacComputer_B.sfcc_1_slats_out = ElacComputer_U.in.discrete_inputs.sfcc_1_slats_out;
    ElacComputer_B.SSM_om = ElacComputer_U.in.bus_inputs.ir_3_bus.inertial_alt_ft.SSM;
    ElacComputer_B.Data_fp = ElacComputer_U.in.bus_inputs.ir_3_bus.inertial_alt_ft.Data;
    ElacComputer_B.SSM_ar = ElacComputer_U.in.bus_inputs.ir_3_bus.along_track_horiz_acc_g.SSM;
    ElacComputer_B.Data_ns = ElacComputer_U.in.bus_inputs.ir_3_bus.along_track_horiz_acc_g.Data;
    ElacComputer_B.SSM_ce = ElacComputer_U.in.bus_inputs.ir_3_bus.cross_track_horiz_acc_g.SSM;
    ElacComputer_B.Data_m3 = ElacComputer_U.in.bus_inputs.ir_3_bus.cross_track_horiz_acc_g.Data;
    ElacComputer_B.SSM_ed = ElacComputer_U.in.bus_inputs.ir_3_bus.vertical_accel_g.SSM;
    ElacComputer_B.Data_oj = ElacComputer_U.in.bus_inputs.ir_3_bus.vertical_accel_g.Data;
    ElacComputer_B.SSM_jh = ElacComputer_U.in.bus_inputs.ir_3_bus.inertial_vertical_speed_ft_s.SSM;
    ElacComputer_B.Data_jy = ElacComputer_U.in.bus_inputs.ir_3_bus.inertial_vertical_speed_ft_s.Data;
    ElacComputer_B.sfcc_2_slats_out = ElacComputer_U.in.discrete_inputs.sfcc_2_slats_out;
    ElacComputer_B.SSM_je = ElacComputer_U.in.bus_inputs.ir_3_bus.north_south_velocity_kn.SSM;
    ElacComputer_B.Data_j1 = ElacComputer_U.in.bus_inputs.ir_3_bus.north_south_velocity_kn.Data;
    ElacComputer_B.SSM_jt = ElacComputer_U.in.bus_inputs.ir_3_bus.east_west_velocity_kn.SSM;
    ElacComputer_B.Data_fc = ElacComputer_U.in.bus_inputs.ir_3_bus.east_west_velocity_kn.Data;
    ElacComputer_B.SSM_cui = ElacComputer_U.in.bus_inputs.fmgc_1_bus.fac_weight_lbs.SSM;
    ElacComputer_B.Data_of = ElacComputer_U.in.bus_inputs.fmgc_1_bus.fac_weight_lbs.Data;
    ElacComputer_B.SSM_mq = ElacComputer_U.in.bus_inputs.fmgc_1_bus.fm_weight_lbs.SSM;
    ElacComputer_B.Data_lg = ElacComputer_U.in.bus_inputs.fmgc_1_bus.fm_weight_lbs.Data;
    ElacComputer_B.SSM_ni = ElacComputer_U.in.bus_inputs.fmgc_1_bus.fac_cg_percent.SSM;
    ElacComputer_B.Data_n4 = ElacComputer_U.in.bus_inputs.fmgc_1_bus.fac_cg_percent.Data;
    ElacComputer_B.l_ail_servo_failed = ElacComputer_U.in.discrete_inputs.l_ail_servo_failed;
    ElacComputer_B.SSM_df = ElacComputer_U.in.bus_inputs.fmgc_1_bus.fm_cg_percent.SSM;
    ElacComputer_B.Data_ot = ElacComputer_U.in.bus_inputs.fmgc_1_bus.fm_cg_percent.Data;
    ElacComputer_B.SSM_oe = ElacComputer_U.in.bus_inputs.fmgc_1_bus.fg_radio_height_ft.SSM;
    ElacComputer_B.Data_gv = ElacComputer_U.in.bus_inputs.fmgc_1_bus.fg_radio_height_ft.Data;
    ElacComputer_B.SSM_ha = ElacComputer_U.in.bus_inputs.fmgc_1_bus.discrete_word_4.SSM;
    ElacComputer_B.Data_ou = ElacComputer_U.in.bus_inputs.fmgc_1_bus.discrete_word_4.Data;
    ElacComputer_B.SSM_op = ElacComputer_U.in.bus_inputs.fmgc_1_bus.ats_discrete_word.SSM;
    ElacComputer_B.Data_dh = ElacComputer_U.in.bus_inputs.fmgc_1_bus.ats_discrete_word.Data;
    ElacComputer_B.SSM_a50 = ElacComputer_U.in.bus_inputs.fmgc_1_bus.discrete_word_3.SSM;
    ElacComputer_B.Data_ph = ElacComputer_U.in.bus_inputs.fmgc_1_bus.discrete_word_3.Data;
    ElacComputer_B.monotonic_time = ElacComputer_U.in.time.monotonic_time;
    ElacComputer_B.l_elev_servo_failed = ElacComputer_U.in.discrete_inputs.l_elev_servo_failed;
    ElacComputer_B.SSM_og = ElacComputer_U.in.bus_inputs.fmgc_1_bus.discrete_word_1.SSM;
    ElacComputer_B.Data_gs = ElacComputer_U.in.bus_inputs.fmgc_1_bus.discrete_word_1.Data;
    ElacComputer_B.SSM_a4 = ElacComputer_U.in.bus_inputs.fmgc_1_bus.discrete_word_2.SSM;
    ElacComputer_B.Data_fd4 = ElacComputer_U.in.bus_inputs.fmgc_1_bus.discrete_word_2.Data;
    ElacComputer_B.SSM_bv = ElacComputer_U.in.bus_inputs.fmgc_1_bus.approach_spd_target_kn.SSM;
    ElacComputer_B.Data_hm = ElacComputer_U.in.bus_inputs.fmgc_1_bus.approach_spd_target_kn.Data;
    ElacComputer_B.SSM_bo = ElacComputer_U.in.bus_inputs.fmgc_1_bus.delta_p_ail_cmd_deg.SSM;
    ElacComputer_B.Data_i2 = ElacComputer_U.in.bus_inputs.fmgc_1_bus.delta_p_ail_cmd_deg.Data;
    ElacComputer_B.SSM_d1 = ElacComputer_U.in.bus_inputs.fmgc_1_bus.delta_p_splr_cmd_deg.SSM;
    ElacComputer_B.Data_og = ElacComputer_U.in.bus_inputs.fmgc_1_bus.delta_p_splr_cmd_deg.Data;
    ElacComputer_B.r_ail_servo_failed = ElacComputer_U.in.discrete_inputs.r_ail_servo_failed;
    ElacComputer_B.SSM_hy = ElacComputer_U.in.bus_inputs.fmgc_1_bus.delta_r_cmd_deg.SSM;
    ElacComputer_B.Data_fv = ElacComputer_U.in.bus_inputs.fmgc_1_bus.delta_r_cmd_deg.Data;
    ElacComputer_B.SSM_gi = ElacComputer_U.in.bus_inputs.fmgc_1_bus.delta_nose_wheel_cmd_deg.SSM;
    ElacComputer_B.Data_oc = ElacComputer_U.in.bus_inputs.fmgc_1_bus.delta_nose_wheel_cmd_deg.Data;
    ElacComputer_B.SSM_pp = ElacComputer_U.in.bus_inputs.fmgc_1_bus.delta_q_cmd_deg.SSM;
    ElacComputer_B.Data_kq = ElacComputer_U.in.bus_inputs.fmgc_1_bus.delta_q_cmd_deg.Data;
    ElacComputer_B.SSM_iab = ElacComputer_U.in.bus_inputs.fmgc_1_bus.n1_left_percent.SSM;
    ElacComputer_B.Data_ne = ElacComputer_U.in.bus_inputs.fmgc_1_bus.n1_left_percent.Data;
    ElacComputer_B.SSM_jtv = ElacComputer_U.in.bus_inputs.fmgc_1_bus.n1_right_percent.SSM;
    ElacComputer_B.Data_it = ElacComputer_U.in.bus_inputs.fmgc_1_bus.n1_right_percent.Data;
    ElacComputer_B.r_elev_servo_failed = ElacComputer_U.in.discrete_inputs.r_elev_servo_failed;
    ElacComputer_B.SSM_fy = ElacComputer_U.in.bus_inputs.fmgc_2_bus.fac_weight_lbs.SSM;
    ElacComputer_B.Data_ch = ElacComputer_U.in.bus_inputs.fmgc_2_bus.fac_weight_lbs.Data;
    ElacComputer_B.SSM_d4 = ElacComputer_U.in.bus_inputs.fmgc_2_bus.fm_weight_lbs.SSM;
    ElacComputer_B.Data_bb = ElacComputer_U.in.bus_inputs.fmgc_2_bus.fm_weight_lbs.Data;
    ElacComputer_B.SSM_ars = ElacComputer_U.in.bus_inputs.fmgc_2_bus.fac_cg_percent.SSM;
    ElacComputer_B.Data_ol = ElacComputer_U.in.bus_inputs.fmgc_2_bus.fac_cg_percent.Data;
    ElacComputer_B.SSM_din = ElacComputer_U.in.bus_inputs.fmgc_2_bus.fm_cg_percent.SSM;
    ElacComputer_B.Data_hw = ElacComputer_U.in.bus_inputs.fmgc_2_bus.fm_cg_percent.Data;
    ElacComputer_B.SSM_m3 = ElacComputer_U.in.bus_inputs.fmgc_2_bus.fg_radio_height_ft.SSM;
    ElacComputer_B.Data_hs = ElacComputer_U.in.bus_inputs.fmgc_2_bus.fg_radio_height_ft.Data;
    ElacComputer_B.ths_override_active = ElacComputer_U.in.discrete_inputs.ths_override_active;
    ElacComputer_B.SSM_np = ElacComputer_U.in.bus_inputs.fmgc_2_bus.discrete_word_4.SSM;
    ElacComputer_B.Data_fj = ElacComputer_U.in.bus_inputs.fmgc_2_bus.discrete_word_4.Data;
    ElacComputer_B.SSM_ax = ElacComputer_U.in.bus_inputs.fmgc_2_bus.ats_discrete_word.SSM;
    ElacComputer_B.Data_ky = ElacComputer_U.in.bus_inputs.fmgc_2_bus.ats_discrete_word.Data;
    ElacComputer_B.SSM_cl = ElacComputer_U.in.bus_inputs.fmgc_2_bus.discrete_word_3.SSM;
    ElacComputer_B.Data_h5 = ElacComputer_U.in.bus_inputs.fmgc_2_bus.discrete_word_3.Data;
    ElacComputer_B.SSM_es = ElacComputer_U.in.bus_inputs.fmgc_2_bus.discrete_word_1.SSM;
    ElacComputer_B.Data_ku = ElacComputer_U.in.bus_inputs.fmgc_2_bus.discrete_word_1.Data;
    ElacComputer_B.SSM_gi1 = ElacComputer_U.in.bus_inputs.fmgc_2_bus.discrete_word_2.SSM;
    ElacComputer_B.Data_jp = ElacComputer_U.in.bus_inputs.fmgc_2_bus.discrete_word_2.Data;
    ElacComputer_B.yellow_low_pressure = ElacComputer_U.in.discrete_inputs.yellow_low_pressure;
    ElacComputer_B.SSM_jz = ElacComputer_U.in.bus_inputs.fmgc_2_bus.approach_spd_target_kn.SSM;
    ElacComputer_B.Data_nu = ElacComputer_U.in.bus_inputs.fmgc_2_bus.approach_spd_target_kn.Data;
    ElacComputer_B.SSM_kt = ElacComputer_U.in.bus_inputs.fmgc_2_bus.delta_p_ail_cmd_deg.SSM;
    ElacComputer_B.Data_br = ElacComputer_U.in.bus_inputs.fmgc_2_bus.delta_p_ail_cmd_deg.Data;
    ElacComputer_B.SSM_ds = ElacComputer_U.in.bus_inputs.fmgc_2_bus.delta_p_splr_cmd_deg.SSM;
    ElacComputer_B.Data_ae = ElacComputer_U.in.bus_inputs.fmgc_2_bus.delta_p_splr_cmd_deg.Data;
    ElacComputer_B.SSM_eg = ElacComputer_U.in.bus_inputs.fmgc_2_bus.delta_r_cmd_deg.SSM;
    ElacComputer_B.Data_pe = ElacComputer_U.in.bus_inputs.fmgc_2_bus.delta_r_cmd_deg.Data;
    ElacComputer_B.SSM_a0 = ElacComputer_U.in.bus_inputs.fmgc_2_bus.delta_nose_wheel_cmd_deg.SSM;
    ElacComputer_B.Data_fy = ElacComputer_U.in.bus_inputs.fmgc_2_bus.delta_nose_wheel_cmd_deg.Data;
    ElacComputer_B.capt_priority_takeover_pressed = ElacComputer_U.in.discrete_inputs.capt_priority_takeover_pressed;
    ElacComputer_B.SSM_cv = ElacComputer_U.in.bus_inputs.fmgc_2_bus.delta_q_cmd_deg.SSM;
    ElacComputer_B.Data_na = ElacComputer_U.in.bus_inputs.fmgc_2_bus.delta_q_cmd_deg.Data;
    ElacComputer_B.SSM_ea = ElacComputer_U.in.bus_inputs.fmgc_2_bus.n1_left_percent.SSM;
    ElacComputer_B.Data_my = ElacComputer_U.in.bus_inputs.fmgc_2_bus.n1_left_percent.Data;
    ElacComputer_B.SSM_p4 = ElacComputer_U.in.bus_inputs.fmgc_2_bus.n1_right_percent.SSM;
    ElacComputer_B.Data_i4 = ElacComputer_U.in.bus_inputs.fmgc_2_bus.n1_right_percent.Data;
    ElacComputer_B.SSM_m2 = ElacComputer_U.in.bus_inputs.ra_1_bus.radio_height_ft.SSM;
    ElacComputer_B.Data_cx = ElacComputer_U.in.bus_inputs.ra_1_bus.radio_height_ft.Data;
    ElacComputer_B.SSM_bt0 = ElacComputer_U.in.bus_inputs.ra_2_bus.radio_height_ft.SSM;
    ElacComputer_B.Data_nz = ElacComputer_U.in.bus_inputs.ra_2_bus.radio_height_ft.Data;
    ElacComputer_B.fo_priority_takeover_pressed = ElacComputer_U.in.discrete_inputs.fo_priority_takeover_pressed;
    ElacComputer_B.SSM_nr = ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_component_status_word.SSM;
    ElacComputer_B.Data_id = ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_component_status_word.Data;
    ElacComputer_B.SSM_fr = ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word.SSM;
    ElacComputer_B.Data_o2 = ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_system_status_word.Data;
    ElacComputer_B.SSM_cc = ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_actual_position_word.SSM;
    ElacComputer_B.Data_gqq = ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_flap_actual_position_word.Data;
    ElacComputer_B.SSM_lm = ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_actual_position_deg.SSM;
    ElacComputer_B.Data_md = ElacComputer_U.in.bus_inputs.sfcc_1_bus.slat_actual_position_deg.Data;
    ElacComputer_B.SSM_mkm = ElacComputer_U.in.bus_inputs.sfcc_1_bus.flap_actual_position_deg.SSM;
    ElacComputer_B.Data_cz = ElacComputer_U.in.bus_inputs.sfcc_1_bus.flap_actual_position_deg.Data;
    ElacComputer_B.blue_low_pressure = ElacComputer_U.in.discrete_inputs.blue_low_pressure;
    ElacComputer_B.SSM_jhd = ElacComputer_U.in.bus_inputs.sfcc_2_bus.slat_flap_component_status_word.SSM;
    ElacComputer_B.Data_pm = ElacComputer_U.in.bus_inputs.sfcc_2_bus.slat_flap_component_status_word.Data;
    ElacComputer_B.SSM_av = ElacComputer_U.in.bus_inputs.sfcc_2_bus.slat_flap_system_status_word.SSM;
    ElacComputer_B.Data_bj = ElacComputer_U.in.bus_inputs.sfcc_2_bus.slat_flap_system_status_word.Data;
    ElacComputer_B.SSM_ira = ElacComputer_U.in.bus_inputs.sfcc_2_bus.slat_flap_actual_position_word.SSM;
    ElacComputer_B.Data_ox = ElacComputer_U.in.bus_inputs.sfcc_2_bus.slat_flap_actual_position_word.Data;
    ElacComputer_B.SSM_ge = ElacComputer_U.in.bus_inputs.sfcc_2_bus.slat_actual_position_deg.SSM;
    ElacComputer_B.Data_pe5 = ElacComputer_U.in.bus_inputs.sfcc_2_bus.slat_actual_position_deg.Data;
    ElacComputer_B.SSM_lv = ElacComputer_U.in.bus_inputs.sfcc_2_bus.flap_actual_position_deg.SSM;
    ElacComputer_B.Data_jj = ElacComputer_U.in.bus_inputs.sfcc_2_bus.flap_actual_position_deg.Data;
    ElacComputer_B.green_low_pressure = ElacComputer_U.in.discrete_inputs.green_low_pressure;
    ElacComputer_B.SSM_cg = ElacComputer_U.in.bus_inputs.fcdc_1_bus.efcs_status_word_1.SSM;
    ElacComputer_B.Data_p5 = ElacComputer_U.in.bus_inputs.fcdc_1_bus.efcs_status_word_1.Data;
    ElacComputer_B.SSM_be = ElacComputer_U.in.bus_inputs.fcdc_1_bus.efcs_status_word_2.SSM;
    ElacComputer_B.Data_ekl = ElacComputer_U.in.bus_inputs.fcdc_1_bus.efcs_status_word_2.Data;
    ElacComputer_B.SSM_axb = ElacComputer_U.in.bus_inputs.fcdc_1_bus.efcs_status_word_3.SSM;
    ElacComputer_B.Data_nd = ElacComputer_U.in.bus_inputs.fcdc_1_bus.efcs_status_word_3.Data;
    ElacComputer_B.SSM_nz = ElacComputer_U.in.bus_inputs.fcdc_1_bus.efcs_status_word_4.SSM;
    ElacComputer_B.Data_n2 = ElacComputer_U.in.bus_inputs.fcdc_1_bus.efcs_status_word_4.Data;
    ElacComputer_B.SSM_cx = ElacComputer_U.in.bus_inputs.fcdc_1_bus.efcs_status_word_5.SSM;
    ElacComputer_B.Data_dl = ElacComputer_U.in.bus_inputs.fcdc_1_bus.efcs_status_word_5.Data;
    ElacComputer_B.elac_engaged_from_switch = ElacComputer_U.in.discrete_inputs.elac_engaged_from_switch;
    ElacComputer_B.SSM_gh = ElacComputer_U.in.bus_inputs.fcdc_1_bus.capt_roll_command_deg.SSM;
    ElacComputer_B.Data_gs2 = ElacComputer_U.in.bus_inputs.fcdc_1_bus.capt_roll_command_deg.Data;
    ElacComputer_B.SSM_ks = ElacComputer_U.in.bus_inputs.fcdc_1_bus.fo_roll_command_deg.SSM;
    ElacComputer_B.Data_h4 = ElacComputer_U.in.bus_inputs.fcdc_1_bus.fo_roll_command_deg.Data;
    ElacComputer_B.SSM_pw = ElacComputer_U.in.bus_inputs.fcdc_1_bus.rudder_pedal_position_deg.SSM;
    ElacComputer_B.Data_e3 = ElacComputer_U.in.bus_inputs.fcdc_1_bus.rudder_pedal_position_deg.Data;
    ElacComputer_B.SSM_fh = ElacComputer_U.in.bus_inputs.fcdc_1_bus.capt_pitch_command_deg.SSM;
    ElacComputer_B.Data_f5h = ElacComputer_U.in.bus_inputs.fcdc_1_bus.capt_pitch_command_deg.Data;
    ElacComputer_B.SSM_gzn = ElacComputer_U.in.bus_inputs.fcdc_1_bus.fo_pitch_command_deg.SSM;
    ElacComputer_B.Data_an = ElacComputer_U.in.bus_inputs.fcdc_1_bus.fo_pitch_command_deg.Data;
    ElacComputer_B.slew_on = ElacComputer_U.in.sim_data.slew_on;
    ElacComputer_B.normal_powersupply_lost = ElacComputer_U.in.discrete_inputs.normal_powersupply_lost;
    ElacComputer_B.SSM_oo = ElacComputer_U.in.bus_inputs.fcdc_1_bus.aileron_left_pos_deg.SSM;
    ElacComputer_B.Data_i4o = ElacComputer_U.in.bus_inputs.fcdc_1_bus.aileron_left_pos_deg.Data;
    ElacComputer_B.SSM_evh = ElacComputer_U.in.bus_inputs.fcdc_1_bus.elevator_left_pos_deg.SSM;
    ElacComputer_B.Data_af = ElacComputer_U.in.bus_inputs.fcdc_1_bus.elevator_left_pos_deg.Data;
    ElacComputer_B.SSM_cn = ElacComputer_U.in.bus_inputs.fcdc_1_bus.aileron_right_pos_deg.SSM;
    ElacComputer_B.Data_bm = ElacComputer_U.in.bus_inputs.fcdc_1_bus.aileron_right_pos_deg.Data;
    ElacComputer_B.SSM_co = ElacComputer_U.in.bus_inputs.fcdc_1_bus.elevator_right_pos_deg.SSM;
    ElacComputer_B.Data_dk = ElacComputer_U.in.bus_inputs.fcdc_1_bus.elevator_right_pos_deg.Data;
    ElacComputer_B.SSM_pe = ElacComputer_U.in.bus_inputs.fcdc_1_bus.horiz_stab_trim_pos_deg.SSM;
    ElacComputer_B.Data_nv = ElacComputer_U.in.bus_inputs.fcdc_1_bus.horiz_stab_trim_pos_deg.Data;
    ElacComputer_B.capt_pitch_stick_pos = ElacComputer_U.in.analog_inputs.capt_pitch_stick_pos;
    ElacComputer_B.SSM_cgz = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_1_left_pos_deg.SSM;
    ElacComputer_B.Data_jpf = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_1_left_pos_deg.Data;
    ElacComputer_B.SSM_fw = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_2_left_pos_deg.SSM;
    ElacComputer_B.Data_i5 = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_2_left_pos_deg.Data;
    ElacComputer_B.SSM_h4 = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_3_left_pos_deg.SSM;
    ElacComputer_B.Data_k2 = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_3_left_pos_deg.Data;
    ElacComputer_B.SSM_cb3 = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_4_left_pos_deg.SSM;
    ElacComputer_B.Data_as = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_4_left_pos_deg.Data;
    ElacComputer_B.SSM_pj = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_5_left_pos_deg.SSM;
    ElacComputer_B.Data_gk = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_5_left_pos_deg.Data;
    ElacComputer_B.fo_pitch_stick_pos = ElacComputer_U.in.analog_inputs.fo_pitch_stick_pos;
    ElacComputer_B.SSM_dv = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_1_right_pos_deg.SSM;
    ElacComputer_B.Data_jl = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_1_right_pos_deg.Data;
    ElacComputer_B.SSM_i4 = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_2_right_pos_deg.SSM;
    ElacComputer_B.Data_e32 = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_2_right_pos_deg.Data;
    ElacComputer_B.SSM_fm = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_3_right_pos_deg.SSM;
    ElacComputer_B.Data_ih = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_3_right_pos_deg.Data;
    ElacComputer_B.SSM_e5 = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_4_right_pos_deg.SSM;
    ElacComputer_B.Data_du = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_4_right_pos_deg.Data;
    ElacComputer_B.SSM_bf = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_5_right_pos_deg.SSM;
    ElacComputer_B.Data_nx = ElacComputer_U.in.bus_inputs.fcdc_1_bus.spoiler_5_right_pos_deg.Data;
    ElacComputer_B.capt_roll_stick_pos = ElacComputer_U.in.analog_inputs.capt_roll_stick_pos;
    ElacComputer_B.SSM_fd = ElacComputer_U.in.bus_inputs.fcdc_2_bus.efcs_status_word_1.SSM;
    ElacComputer_B.Data_n0 = ElacComputer_U.in.bus_inputs.fcdc_2_bus.efcs_status_word_1.Data;
    ElacComputer_B.SSM_fv = ElacComputer_U.in.bus_inputs.fcdc_2_bus.efcs_status_word_2.SSM;
    ElacComputer_B.Data_eqi = ElacComputer_U.in.bus_inputs.fcdc_2_bus.efcs_status_word_2.Data;
    ElacComputer_B.SSM_dt = ElacComputer_U.in.bus_inputs.fcdc_2_bus.efcs_status_word_3.SSM;
    ElacComputer_B.Data_om = ElacComputer_U.in.bus_inputs.fcdc_2_bus.efcs_status_word_3.Data;
    ElacComputer_B.SSM_j5 = ElacComputer_U.in.bus_inputs.fcdc_2_bus.efcs_status_word_4.SSM;
    ElacComputer_B.Data_nr = ElacComputer_U.in.bus_inputs.fcdc_2_bus.efcs_status_word_4.Data;
    ElacComputer_B.SSM_ng = ElacComputer_U.in.bus_inputs.fcdc_2_bus.efcs_status_word_5.SSM;
    ElacComputer_B.Data_p3 = ElacComputer_U.in.bus_inputs.fcdc_2_bus.efcs_status_word_5.Data;
    ElacComputer_B.fo_roll_stick_pos = ElacComputer_U.in.analog_inputs.fo_roll_stick_pos;
    ElacComputer_B.SSM_cs = ElacComputer_U.in.bus_inputs.fcdc_2_bus.capt_roll_command_deg.SSM;
    ElacComputer_B.Data_nb = ElacComputer_U.in.bus_inputs.fcdc_2_bus.capt_roll_command_deg.Data;
    ElacComputer_B.SSM_ls = ElacComputer_U.in.bus_inputs.fcdc_2_bus.fo_roll_command_deg.SSM;
    ElacComputer_B.Data_hd = ElacComputer_U.in.bus_inputs.fcdc_2_bus.fo_roll_command_deg.Data;
    ElacComputer_B.SSM_dg = ElacComputer_U.in.bus_inputs.fcdc_2_bus.rudder_pedal_position_deg.SSM;
    ElacComputer_B.Data_al = ElacComputer_U.in.bus_inputs.fcdc_2_bus.rudder_pedal_position_deg.Data;
    ElacComputer_B.SSM_d3 = ElacComputer_U.in.bus_inputs.fcdc_2_bus.capt_pitch_command_deg.SSM;
    ElacComputer_B.Data_gu = ElacComputer_U.in.bus_inputs.fcdc_2_bus.capt_pitch_command_deg.Data;
    ElacComputer_B.SSM_p2 = ElacComputer_U.in.bus_inputs.fcdc_2_bus.fo_pitch_command_deg.SSM;
    ElacComputer_B.Data_ix = ElacComputer_U.in.bus_inputs.fcdc_2_bus.fo_pitch_command_deg.Data;
    ElacComputer_B.left_elevator_pos_deg = ElacComputer_U.in.analog_inputs.left_elevator_pos_deg;
    ElacComputer_B.SSM_bo0 = ElacComputer_U.in.bus_inputs.fcdc_2_bus.aileron_left_pos_deg.SSM;
    ElacComputer_B.Data_do = ElacComputer_U.in.bus_inputs.fcdc_2_bus.aileron_left_pos_deg.Data;
    ElacComputer_B.SSM_bc = ElacComputer_U.in.bus_inputs.fcdc_2_bus.elevator_left_pos_deg.SSM;
    ElacComputer_B.Data_hu = ElacComputer_U.in.bus_inputs.fcdc_2_bus.elevator_left_pos_deg.Data;
    ElacComputer_B.SSM_h0 = ElacComputer_U.in.bus_inputs.fcdc_2_bus.aileron_right_pos_deg.SSM;
    ElacComputer_B.Data_pm1 = ElacComputer_U.in.bus_inputs.fcdc_2_bus.aileron_right_pos_deg.Data;
    ElacComputer_B.SSM_giz = ElacComputer_U.in.bus_inputs.fcdc_2_bus.elevator_right_pos_deg.SSM;
    ElacComputer_B.Data_i2y = ElacComputer_U.in.bus_inputs.fcdc_2_bus.elevator_right_pos_deg.Data;
    ElacComputer_B.SSM_mqp = ElacComputer_U.in.bus_inputs.fcdc_2_bus.horiz_stab_trim_pos_deg.SSM;
    ElacComputer_B.Data_pg = ElacComputer_U.in.bus_inputs.fcdc_2_bus.horiz_stab_trim_pos_deg.Data;
    ElacComputer_B.right_elevator_pos_deg = ElacComputer_U.in.analog_inputs.right_elevator_pos_deg;
    ElacComputer_B.SSM_ba = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_1_left_pos_deg.SSM;
    ElacComputer_B.Data_ni = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_1_left_pos_deg.Data;
    ElacComputer_B.SSM_in = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_2_left_pos_deg.SSM;
    ElacComputer_B.Data_fr = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_2_left_pos_deg.Data;
    ElacComputer_B.SSM_ff = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_3_left_pos_deg.SSM;
    ElacComputer_B.Data_cn = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_3_left_pos_deg.Data;
    ElacComputer_B.SSM_ic = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_4_left_pos_deg.SSM;
    ElacComputer_B.Data_nxl = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_4_left_pos_deg.Data;
    ElacComputer_B.SSM_fs = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_5_left_pos_deg.SSM;
    ElacComputer_B.Data_jh = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_5_left_pos_deg.Data;
    ElacComputer_B.ths_pos_deg = ElacComputer_U.in.analog_inputs.ths_pos_deg;
    ElacComputer_B.SSM_ja = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_1_right_pos_deg.SSM;
    ElacComputer_B.Data_gl = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_1_right_pos_deg.Data;
    ElacComputer_B.SSM_js = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_2_right_pos_deg.SSM;
    ElacComputer_B.Data_gn = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_2_right_pos_deg.Data;
    ElacComputer_B.SSM_is3 = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_3_right_pos_deg.SSM;
    ElacComputer_B.Data_myb = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_3_right_pos_deg.Data;
    ElacComputer_B.SSM_ag = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_4_right_pos_deg.SSM;
    ElacComputer_B.Data_l2 = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_4_right_pos_deg.Data;
    ElacComputer_B.SSM_f5 = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_5_right_pos_deg.SSM;
    ElacComputer_B.Data_o5o = ElacComputer_U.in.bus_inputs.fcdc_2_bus.spoiler_5_right_pos_deg.Data;
    ElacComputer_B.left_aileron_pos_deg = ElacComputer_U.in.analog_inputs.left_aileron_pos_deg;
    ElacComputer_B.SSM_ph = ElacComputer_U.in.bus_inputs.sec_1_bus.left_spoiler_1_position_deg.SSM;
    ElacComputer_B.Data_l5 = ElacComputer_U.in.bus_inputs.sec_1_bus.left_spoiler_1_position_deg.Data;
    ElacComputer_B.SSM_jw = ElacComputer_U.in.bus_inputs.sec_1_bus.right_spoiler_1_position_deg.SSM;
    ElacComputer_B.Data_dc2 = ElacComputer_U.in.bus_inputs.sec_1_bus.right_spoiler_1_position_deg.Data;
    ElacComputer_B.SSM_jy = ElacComputer_U.in.bus_inputs.sec_1_bus.left_spoiler_2_position_deg.SSM;
    ElacComputer_B.Data_gr = ElacComputer_U.in.bus_inputs.sec_1_bus.left_spoiler_2_position_deg.Data;
    ElacComputer_B.SSM_j1 = ElacComputer_U.in.bus_inputs.sec_1_bus.right_spoiler_2_position_deg.SSM;
    ElacComputer_B.Data_gp = ElacComputer_U.in.bus_inputs.sec_1_bus.right_spoiler_2_position_deg.Data;
    ElacComputer_B.SSM_ov = ElacComputer_U.in.bus_inputs.sec_1_bus.left_elevator_position_deg.SSM;
    ElacComputer_B.Data_i3 = ElacComputer_U.in.bus_inputs.sec_1_bus.left_elevator_position_deg.Data;
    ElacComputer_B.right_aileron_pos_deg = ElacComputer_U.in.analog_inputs.right_aileron_pos_deg;
    ElacComputer_B.SSM_mx = ElacComputer_U.in.bus_inputs.sec_1_bus.right_elevator_position_deg.SSM;
    ElacComputer_B.Data_et = ElacComputer_U.in.bus_inputs.sec_1_bus.right_elevator_position_deg.Data;
    ElacComputer_B.SSM_b4 = ElacComputer_U.in.bus_inputs.sec_1_bus.ths_position_deg.SSM;
    ElacComputer_B.Data_mc = ElacComputer_U.in.bus_inputs.sec_1_bus.ths_position_deg.Data;
    ElacComputer_B.SSM_gb = ElacComputer_U.in.bus_inputs.sec_1_bus.left_sidestick_pitch_command_deg.SSM;
    ElacComputer_B.Data_k3 = ElacComputer_U.in.bus_inputs.sec_1_bus.left_sidestick_pitch_command_deg.Data;
    ElacComputer_B.SSM_oh = ElacComputer_U.in.bus_inputs.sec_1_bus.right_sidestick_pitch_command_deg.SSM;
    ElacComputer_B.Data_f2 = ElacComputer_U.in.bus_inputs.sec_1_bus.right_sidestick_pitch_command_deg.Data;
    ElacComputer_B.SSM_mm5 = ElacComputer_U.in.bus_inputs.sec_1_bus.left_sidestick_roll_command_deg.SSM;
    ElacComputer_B.Data_gh = ElacComputer_U.in.bus_inputs.sec_1_bus.left_sidestick_roll_command_deg.Data;
    ElacComputer_B.pause_on = ElacComputer_U.in.sim_data.pause_on;
    ElacComputer_B.rudder_pedal_pos = ElacComputer_U.in.analog_inputs.rudder_pedal_pos;
    ElacComputer_B.SSM_br = ElacComputer_U.in.bus_inputs.sec_1_bus.right_sidestick_roll_command_deg.SSM;
    ElacComputer_B.Data_ed = ElacComputer_U.in.bus_inputs.sec_1_bus.right_sidestick_roll_command_deg.Data;
    ElacComputer_B.SSM_c2 = ElacComputer_U.in.bus_inputs.sec_1_bus.speed_brake_lever_command_deg.SSM;
    ElacComputer_B.Data_o2j = ElacComputer_U.in.bus_inputs.sec_1_bus.speed_brake_lever_command_deg.Data;
    ElacComputer_B.SSM_hc = ElacComputer_U.in.bus_inputs.sec_1_bus.thrust_lever_angle_1_deg.SSM;
    ElacComputer_B.Data_i43 = ElacComputer_U.in.bus_inputs.sec_1_bus.thrust_lever_angle_1_deg.Data;
    ElacComputer_B.SSM_ktr = ElacComputer_U.in.bus_inputs.sec_1_bus.thrust_lever_angle_2_deg.SSM;
    ElacComputer_B.Data_ic = ElacComputer_U.in.bus_inputs.sec_1_bus.thrust_lever_angle_2_deg.Data;
    ElacComputer_B.SSM_gl = ElacComputer_U.in.bus_inputs.sec_1_bus.discrete_status_word_1.SSM;
    ElacComputer_B.Data_ak = ElacComputer_U.in.bus_inputs.sec_1_bus.discrete_status_word_1.Data;
    ElacComputer_B.load_factor_acc_1_g = ElacComputer_U.in.analog_inputs.load_factor_acc_1_g;
    ElacComputer_B.SSM_my = ElacComputer_U.in.bus_inputs.sec_1_bus.discrete_status_word_2.SSM;
    ElacComputer_B.Data_jg = ElacComputer_U.in.bus_inputs.sec_1_bus.discrete_status_word_2.Data;
    ElacComputer_B.SSM_j3 = ElacComputer_U.in.bus_inputs.sec_2_bus.left_spoiler_1_position_deg.SSM;
    ElacComputer_B.Data_cu = ElacComputer_U.in.bus_inputs.sec_2_bus.left_spoiler_1_position_deg.Data;
    ElacComputer_B.SSM_go = ElacComputer_U.in.bus_inputs.sec_2_bus.right_spoiler_1_position_deg.SSM;
    ElacComputer_B.Data_ep = ElacComputer_U.in.bus_inputs.sec_2_bus.right_spoiler_1_position_deg.Data;
    ElacComputer_B.SSM_e5c = ElacComputer_U.in.bus_inputs.sec_2_bus.left_spoiler_2_position_deg.SSM;
    ElacComputer_B.Data_d3 = ElacComputer_U.in.bus_inputs.sec_2_bus.left_spoiler_2_position_deg.Data;
    ElacComputer_B.SSM_dk = ElacComputer_U.in.bus_inputs.sec_2_bus.right_spoiler_2_position_deg.SSM;
    ElacComputer_B.Data_bt = ElacComputer_U.in.bus_inputs.sec_2_bus.right_spoiler_2_position_deg.Data;
    ElacComputer_B.load_factor_acc_2_g = ElacComputer_U.in.analog_inputs.load_factor_acc_2_g;
    ElacComputer_B.SSM_evc = ElacComputer_U.in.bus_inputs.sec_2_bus.left_elevator_position_deg.SSM;
    ElacComputer_B.Data_e0 = ElacComputer_U.in.bus_inputs.sec_2_bus.left_elevator_position_deg.Data;
    ElacComputer_B.SSM_kk = ElacComputer_U.in.bus_inputs.sec_2_bus.right_elevator_position_deg.SSM;
    ElacComputer_B.Data_jl3 = ElacComputer_U.in.bus_inputs.sec_2_bus.right_elevator_position_deg.Data;
    ElacComputer_B.SSM_af = ElacComputer_U.in.bus_inputs.sec_2_bus.ths_position_deg.SSM;
    ElacComputer_B.Data_nm = ElacComputer_U.in.bus_inputs.sec_2_bus.ths_position_deg.Data;
    ElacComputer_B.SSM_npr = ElacComputer_U.in.bus_inputs.sec_2_bus.left_sidestick_pitch_command_deg.SSM;
    ElacComputer_B.Data_ia = ElacComputer_U.in.bus_inputs.sec_2_bus.left_sidestick_pitch_command_deg.Data;
    ElacComputer_B.SSM_ew = ElacComputer_U.in.bus_inputs.sec_2_bus.right_sidestick_pitch_command_deg.SSM;
    ElacComputer_B.Data_j0 = ElacComputer_U.in.bus_inputs.sec_2_bus.right_sidestick_pitch_command_deg.Data;
    ElacComputer_B.blue_hyd_pressure_psi = ElacComputer_U.in.analog_inputs.blue_hyd_pressure_psi;
    ElacComputer_B.SSM_lt = ElacComputer_U.in.bus_inputs.sec_2_bus.left_sidestick_roll_command_deg.SSM;
    ElacComputer_B.Data_bs = ElacComputer_U.in.bus_inputs.sec_2_bus.left_sidestick_roll_command_deg.Data;
    ElacComputer_B.SSM_ger = ElacComputer_U.in.bus_inputs.sec_2_bus.right_sidestick_roll_command_deg.SSM;
    ElacComputer_B.Data_hp = ElacComputer_U.in.bus_inputs.sec_2_bus.right_sidestick_roll_command_deg.Data;
    ElacComputer_B.SSM_pxo = ElacComputer_U.in.bus_inputs.sec_2_bus.speed_brake_lever_command_deg.SSM;
    ElacComputer_B.Data_ct = ElacComputer_U.in.bus_inputs.sec_2_bus.speed_brake_lever_command_deg.Data;
    ElacComputer_B.SSM_co2 = ElacComputer_U.in.bus_inputs.sec_2_bus.thrust_lever_angle_1_deg.SSM;
    ElacComputer_B.Data_pc = ElacComputer_U.in.bus_inputs.sec_2_bus.thrust_lever_angle_1_deg.Data;
    ElacComputer_B.SSM_ny = ElacComputer_U.in.bus_inputs.sec_2_bus.thrust_lever_angle_2_deg.SSM;
    ElacComputer_B.Data_nzt = ElacComputer_U.in.bus_inputs.sec_2_bus.thrust_lever_angle_2_deg.Data;
    ElacComputer_B.green_hyd_pressure_psi = ElacComputer_U.in.analog_inputs.green_hyd_pressure_psi;
    ElacComputer_B.SSM_l4 = ElacComputer_U.in.bus_inputs.sec_2_bus.discrete_status_word_1.SSM;
    ElacComputer_B.Data_c0 = ElacComputer_U.in.bus_inputs.sec_2_bus.discrete_status_word_1.Data;
    ElacComputer_B.SSM_nm = ElacComputer_U.in.bus_inputs.sec_2_bus.discrete_status_word_2.SSM;
    ElacComputer_B.Data_ojg = ElacComputer_U.in.bus_inputs.sec_2_bus.discrete_status_word_2.Data;
    ElacComputer_B.SSM_nh = ElacComputer_U.in.bus_inputs.elac_opp_bus.left_aileron_position_deg.SSM;
    ElacComputer_B.Data_lm = ElacComputer_U.in.bus_inputs.elac_opp_bus.left_aileron_position_deg.Data;
    ElacComputer_B.SSM_dl = ElacComputer_U.in.bus_inputs.elac_opp_bus.right_aileron_position_deg.SSM;
    ElacComputer_B.Data_fz = ElacComputer_U.in.bus_inputs.elac_opp_bus.right_aileron_position_deg.Data;
    ElacComputer_B.SSM_dx = ElacComputer_U.in.bus_inputs.elac_opp_bus.left_elevator_position_deg.SSM;
    ElacComputer_B.Data_oz = ElacComputer_U.in.bus_inputs.elac_opp_bus.left_elevator_position_deg.Data;
    ElacComputer_B.yellow_hyd_pressure_psi = ElacComputer_U.in.analog_inputs.yellow_hyd_pressure_psi;
    ElacComputer_B.SSM_a5h = ElacComputer_U.in.bus_inputs.elac_opp_bus.right_elevator_position_deg.SSM;
    ElacComputer_B.Data_gf = ElacComputer_U.in.bus_inputs.elac_opp_bus.right_elevator_position_deg.Data;
    ElacComputer_B.SSM_fl = ElacComputer_U.in.bus_inputs.elac_opp_bus.ths_position_deg.SSM;
    ElacComputer_B.Data_nn = ElacComputer_U.in.bus_inputs.elac_opp_bus.ths_position_deg.Data;
    ElacComputer_B.SSM_p3 = ElacComputer_U.in.bus_inputs.elac_opp_bus.left_sidestick_pitch_command_deg.SSM;
    ElacComputer_B.Data_a0z = ElacComputer_U.in.bus_inputs.elac_opp_bus.left_sidestick_pitch_command_deg.Data;
    ElacComputer_B.SSM_ns = ElacComputer_U.in.bus_inputs.elac_opp_bus.right_sidestick_pitch_command_deg.SSM;
    ElacComputer_B.Data_fk = ElacComputer_U.in.bus_inputs.elac_opp_bus.right_sidestick_pitch_command_deg.Data;
    ElacComputer_B.SSM_bm = ElacComputer_U.in.bus_inputs.elac_opp_bus.left_sidestick_roll_command_deg.SSM;
    ElacComputer_B.Data_bu = ElacComputer_U.in.bus_inputs.elac_opp_bus.left_sidestick_roll_command_deg.Data;
    ElacComputer_B.SSM_nl = ElacComputer_U.in.bus_inputs.adr_1_bus.altitude_standard_ft.SSM;
    ElacComputer_B.SSM_grm = ElacComputer_U.in.bus_inputs.elac_opp_bus.right_sidestick_roll_command_deg.SSM;
    ElacComputer_B.Data_o23 = ElacComputer_U.in.bus_inputs.elac_opp_bus.right_sidestick_roll_command_deg.Data;
    ElacComputer_B.SSM_gzm = ElacComputer_U.in.bus_inputs.elac_opp_bus.rudder_pedal_position_deg.SSM;
    ElacComputer_B.Data_g3 = ElacComputer_U.in.bus_inputs.elac_opp_bus.rudder_pedal_position_deg.Data;
    ElacComputer_B.SSM_oi = ElacComputer_U.in.bus_inputs.elac_opp_bus.aileron_command_deg.SSM;
    ElacComputer_B.Data_icc = ElacComputer_U.in.bus_inputs.elac_opp_bus.aileron_command_deg.Data;
    ElacComputer_B.SSM_aa = ElacComputer_U.in.bus_inputs.elac_opp_bus.roll_spoiler_command_deg.SSM;
    ElacComputer_B.Data_pwf = ElacComputer_U.in.bus_inputs.elac_opp_bus.roll_spoiler_command_deg.Data;
    ElacComputer_B.SSM_fvk = ElacComputer_U.in.bus_inputs.elac_opp_bus.yaw_damper_command_deg.SSM;
    ElacComputer_B.Data_gvk = ElacComputer_U.in.bus_inputs.elac_opp_bus.yaw_damper_command_deg.Data;
    ElacComputer_B.Data_ln = ElacComputer_U.in.bus_inputs.adr_1_bus.altitude_standard_ft.Data;
    ElacComputer_B.SSM_lw = ElacComputer_U.in.bus_inputs.elac_opp_bus.elevator_double_pressurization_command_deg.SSM;
    ElacComputer_B.Data_ka = ElacComputer_U.in.bus_inputs.elac_opp_bus.elevator_double_pressurization_command_deg.Data;
    ElacComputer_B.SSM_fa = ElacComputer_U.in.bus_inputs.elac_opp_bus.discrete_status_word_1.SSM;
    ElacComputer_B.Data_mp = ElacComputer_U.in.bus_inputs.elac_opp_bus.discrete_status_word_1.Data;
    ElacComputer_B.SSM_lbx = ElacComputer_U.in.bus_inputs.elac_opp_bus.discrete_status_word_2.SSM;
    ElacComputer_B.Data_m4 = ElacComputer_U.in.bus_inputs.elac_opp_bus.discrete_status_word_2.Data;
    ElacComputer_B.SSM_n3 = ElacComputer_U.in.bus_inputs.adr_1_bus.altitude_corrected_ft.SSM;
    ElacComputer_B.Data_fki = ElacComputer_U.in.bus_inputs.adr_1_bus.altitude_corrected_ft.Data;
    ElacComputer_B.tracking_mode_on_override = ElacComputer_U.in.sim_data.tracking_mode_on_override;
    ElacComputer_B.SSM_a1 = ElacComputer_U.in.bus_inputs.adr_1_bus.mach.SSM;
    ElacComputer_B.Data_bv = ElacComputer_U.in.bus_inputs.adr_1_bus.mach.Data;
    ElacComputer_B.SSM_p1 = ElacComputer_U.in.bus_inputs.adr_1_bus.airspeed_computed_kn.SSM;
    ElacComputer_B.Data_m21 = ElacComputer_U.in.bus_inputs.adr_1_bus.airspeed_computed_kn.Data;
    ElacComputer_B.SSM_cn2 = ElacComputer_U.in.bus_inputs.adr_1_bus.airspeed_true_kn.SSM;
    ElacComputer_B.Data_nbg = ElacComputer_U.in.bus_inputs.adr_1_bus.airspeed_true_kn.Data;
    ElacComputer_B.SSM_an3 = ElacComputer_U.in.bus_inputs.adr_1_bus.vertical_speed_ft_min.SSM;
    ElacComputer_B.Data_l25 = ElacComputer_U.in.bus_inputs.adr_1_bus.vertical_speed_ft_min.Data;
    ElacComputer_B.SSM_c3 = ElacComputer_U.in.bus_inputs.adr_1_bus.aoa_corrected_deg.SSM;
    ElacComputer_B.Data_ki = ElacComputer_U.in.bus_inputs.adr_1_bus.aoa_corrected_deg.Data;
    ElacComputer_B.tailstrike_protection_on = ElacComputer_U.in.sim_data.tailstrike_protection_on;
    ElacComputer_B.SSM_dp = ElacComputer_U.in.bus_inputs.adr_2_bus.altitude_standard_ft.SSM;
    ElacComputer_B.Data_p5p = ElacComputer_U.in.bus_inputs.adr_2_bus.altitude_standard_ft.Data;
    ElacComputer_B.SSM_boy = ElacComputer_U.in.bus_inputs.adr_2_bus.altitude_corrected_ft.SSM;
    ElacComputer_B.Data_nry = ElacComputer_U.in.bus_inputs.adr_2_bus.altitude_corrected_ft.Data;
    ElacComputer_B.SSM_lg = ElacComputer_U.in.bus_inputs.adr_2_bus.mach.SSM;
    ElacComputer_B.Data_mh = ElacComputer_U.in.bus_inputs.adr_2_bus.mach.Data;
    ElacComputer_B.SSM_cm = ElacComputer_U.in.bus_inputs.adr_2_bus.airspeed_computed_kn.SSM;
    ElacComputer_B.Data_ll = ElacComputer_U.in.bus_inputs.adr_2_bus.airspeed_computed_kn.Data;
    ElacComputer_B.SSM_hl = ElacComputer_U.in.bus_inputs.adr_2_bus.airspeed_true_kn.SSM;
    ElacComputer_B.Data_hy = ElacComputer_U.in.bus_inputs.adr_2_bus.airspeed_true_kn.Data;
    ElacComputer_B.computer_running = ElacComputer_U.in.sim_data.computer_running;
    ElacComputer_B.SSM_irh = ElacComputer_U.in.bus_inputs.adr_2_bus.vertical_speed_ft_min.SSM;
    ElacComputer_B.Data_j04 = ElacComputer_U.in.bus_inputs.adr_2_bus.vertical_speed_ft_min.Data;
    ElacComputer_B.SSM_b42 = ElacComputer_U.in.bus_inputs.adr_2_bus.aoa_corrected_deg.SSM;
    ElacComputer_B.Data_pf = ElacComputer_U.in.bus_inputs.adr_2_bus.aoa_corrected_deg.Data;
    ElacComputer_B.SSM_anz = ElacComputer_U.in.bus_inputs.adr_3_bus.altitude_standard_ft.SSM;
    ElacComputer_B.Data_pl = ElacComputer_U.in.bus_inputs.adr_3_bus.altitude_standard_ft.Data;
    ElacComputer_B.SSM_d2 = ElacComputer_U.in.bus_inputs.adr_3_bus.altitude_corrected_ft.SSM;
    ElacComputer_B.Data_gb = ElacComputer_U.in.bus_inputs.adr_3_bus.altitude_corrected_ft.Data;
    ElacComputer_B.SSM_gov = ElacComputer_U.in.bus_inputs.adr_3_bus.mach.SSM;
    ElacComputer_B.Data_hq = ElacComputer_U.in.bus_inputs.adr_3_bus.mach.Data;
    ElacComputer_B.ground_spoilers_active_1 = ElacComputer_U.in.discrete_inputs.ground_spoilers_active_1;
    ElacComputer_B.SSM_nb = ElacComputer_U.in.bus_inputs.adr_3_bus.airspeed_computed_kn.SSM;
    ElacComputer_B.Data_ai = ElacComputer_U.in.bus_inputs.adr_3_bus.airspeed_computed_kn.Data;
    ElacComputer_B.SSM_pe3 = ElacComputer_U.in.bus_inputs.adr_3_bus.airspeed_true_kn.SSM;
    ElacComputer_B.Data_gfr = ElacComputer_U.in.bus_inputs.adr_3_bus.airspeed_true_kn.Data;
    ElacComputer_B.SSM_jj = ElacComputer_U.in.bus_inputs.adr_3_bus.vertical_speed_ft_min.SSM;
    ElacComputer_B.Data_czp = ElacComputer_U.in.bus_inputs.adr_3_bus.vertical_speed_ft_min.Data;
    ElacComputer_B.SSM_jx = ElacComputer_U.in.bus_inputs.adr_3_bus.aoa_corrected_deg.SSM;
    ElacComputer_B.Data_fm = ElacComputer_U.in.bus_inputs.adr_3_bus.aoa_corrected_deg.Data;
    ElacComputer_B.SSM_npl = ElacComputer_U.in.bus_inputs.ir_1_bus.discrete_word_1.SSM;
    ElacComputer_B.Data_jsg = ElacComputer_U.in.bus_inputs.ir_1_bus.discrete_word_1.Data;
    ElacComputer_DWork.Delay_DSTATE = rtb_Y;
    ElacComputer_DWork.icLoad = false;
    ElacComputer_DWork.Delay_DSTATE_c = ElacComputer_DWork.Delay_DSTATE_b;
  } else {
    ElacComputer_DWork.Runtime_MODE = false;
  }

  ElacComputer_Y.out.data.time.dt = ElacComputer_B.dt;
  ElacComputer_Y.out.data.time.simulation_time = ElacComputer_B.simulation_time;
  ElacComputer_Y.out.data.time.monotonic_time = ElacComputer_B.monotonic_time;
  ElacComputer_Y.out.data.sim_data.slew_on = ElacComputer_B.slew_on;
  ElacComputer_Y.out.data.sim_data.pause_on = ElacComputer_B.pause_on;
  ElacComputer_Y.out.data.sim_data.tracking_mode_on_override = ElacComputer_B.tracking_mode_on_override;
  ElacComputer_Y.out.data.sim_data.tailstrike_protection_on = ElacComputer_B.tailstrike_protection_on;
  ElacComputer_Y.out.data.sim_data.computer_running = ElacComputer_B.computer_running;
  ElacComputer_Y.out.data.discrete_inputs.ground_spoilers_active_1 = ElacComputer_B.ground_spoilers_active_1;
  ElacComputer_Y.out.data.discrete_inputs.ground_spoilers_active_2 = ElacComputer_B.ground_spoilers_active_2;
  ElacComputer_Y.out.data.discrete_inputs.is_unit_1 = ElacComputer_B.is_unit_1;
  ElacComputer_Y.out.data.discrete_inputs.is_unit_2 = ElacComputer_B.is_unit_2;
  ElacComputer_Y.out.data.discrete_inputs.opp_axis_pitch_failure = ElacComputer_B.opp_axis_pitch_failure;
  ElacComputer_Y.out.data.discrete_inputs.ap_1_disengaged = ElacComputer_B.ap_1_disengaged;
  ElacComputer_Y.out.data.discrete_inputs.ap_2_disengaged = ElacComputer_B.ap_2_disengaged;
  ElacComputer_Y.out.data.discrete_inputs.opp_left_aileron_lost = ElacComputer_B.opp_left_aileron_lost;
  ElacComputer_Y.out.data.discrete_inputs.opp_right_aileron_lost = ElacComputer_B.opp_right_aileron_lost;
  ElacComputer_Y.out.data.discrete_inputs.fac_1_yaw_control_lost = ElacComputer_B.fac_1_yaw_control_lost;
  ElacComputer_Y.out.data.discrete_inputs.lgciu_1_nose_gear_pressed = ElacComputer_B.lgciu_1_nose_gear_pressed;
  ElacComputer_Y.out.data.discrete_inputs.lgciu_2_nose_gear_pressed = ElacComputer_B.lgciu_2_nose_gear_pressed;
  ElacComputer_Y.out.data.discrete_inputs.fac_2_yaw_control_lost = ElacComputer_B.fac_2_yaw_control_lost;
  ElacComputer_Y.out.data.discrete_inputs.lgciu_1_right_main_gear_pressed =
    ElacComputer_B.lgciu_1_right_main_gear_pressed;
  ElacComputer_Y.out.data.discrete_inputs.lgciu_2_right_main_gear_pressed =
    ElacComputer_B.lgciu_2_right_main_gear_pressed;
  ElacComputer_Y.out.data.discrete_inputs.lgciu_1_left_main_gear_pressed = ElacComputer_B.lgciu_1_left_main_gear_pressed;
  ElacComputer_Y.out.data.discrete_inputs.lgciu_2_left_main_gear_pressed = ElacComputer_B.lgciu_2_left_main_gear_pressed;
  ElacComputer_Y.out.data.discrete_inputs.ths_motor_fault = ElacComputer_B.ths_motor_fault;
  ElacComputer_Y.out.data.discrete_inputs.sfcc_1_slats_out = ElacComputer_B.sfcc_1_slats_out;
  ElacComputer_Y.out.data.discrete_inputs.sfcc_2_slats_out = ElacComputer_B.sfcc_2_slats_out;
  ElacComputer_Y.out.data.discrete_inputs.l_ail_servo_failed = ElacComputer_B.l_ail_servo_failed;
  ElacComputer_Y.out.data.discrete_inputs.l_elev_servo_failed = ElacComputer_B.l_elev_servo_failed;
  ElacComputer_Y.out.data.discrete_inputs.r_ail_servo_failed = ElacComputer_B.r_ail_servo_failed;
  ElacComputer_Y.out.data.discrete_inputs.r_elev_servo_failed = ElacComputer_B.r_elev_servo_failed;
  ElacComputer_Y.out.data.discrete_inputs.ths_override_active = ElacComputer_B.ths_override_active;
  ElacComputer_Y.out.data.discrete_inputs.yellow_low_pressure = ElacComputer_B.yellow_low_pressure;
  ElacComputer_Y.out.data.discrete_inputs.capt_priority_takeover_pressed = ElacComputer_B.capt_priority_takeover_pressed;
  ElacComputer_Y.out.data.discrete_inputs.fo_priority_takeover_pressed = ElacComputer_B.fo_priority_takeover_pressed;
  ElacComputer_Y.out.data.discrete_inputs.blue_low_pressure = ElacComputer_B.blue_low_pressure;
  ElacComputer_Y.out.data.discrete_inputs.green_low_pressure = ElacComputer_B.green_low_pressure;
  ElacComputer_Y.out.data.discrete_inputs.elac_engaged_from_switch = ElacComputer_B.elac_engaged_from_switch;
  ElacComputer_Y.out.data.discrete_inputs.normal_powersupply_lost = ElacComputer_B.normal_powersupply_lost;
  ElacComputer_Y.out.data.analog_inputs.capt_pitch_stick_pos = ElacComputer_B.capt_pitch_stick_pos;
  ElacComputer_Y.out.data.analog_inputs.fo_pitch_stick_pos = ElacComputer_B.fo_pitch_stick_pos;
  ElacComputer_Y.out.data.analog_inputs.capt_roll_stick_pos = ElacComputer_B.capt_roll_stick_pos;
  ElacComputer_Y.out.data.analog_inputs.fo_roll_stick_pos = ElacComputer_B.fo_roll_stick_pos;
  ElacComputer_Y.out.data.analog_inputs.left_elevator_pos_deg = ElacComputer_B.left_elevator_pos_deg;
  ElacComputer_Y.out.data.analog_inputs.right_elevator_pos_deg = ElacComputer_B.right_elevator_pos_deg;
  ElacComputer_Y.out.data.analog_inputs.ths_pos_deg = ElacComputer_B.ths_pos_deg;
  ElacComputer_Y.out.data.analog_inputs.left_aileron_pos_deg = ElacComputer_B.left_aileron_pos_deg;
  ElacComputer_Y.out.data.analog_inputs.right_aileron_pos_deg = ElacComputer_B.right_aileron_pos_deg;
  ElacComputer_Y.out.data.analog_inputs.rudder_pedal_pos = ElacComputer_B.rudder_pedal_pos;
  ElacComputer_Y.out.data.analog_inputs.load_factor_acc_1_g = ElacComputer_B.load_factor_acc_1_g;
  ElacComputer_Y.out.data.analog_inputs.load_factor_acc_2_g = ElacComputer_B.load_factor_acc_2_g;
  ElacComputer_Y.out.data.analog_inputs.blue_hyd_pressure_psi = ElacComputer_B.blue_hyd_pressure_psi;
  ElacComputer_Y.out.data.analog_inputs.green_hyd_pressure_psi = ElacComputer_B.green_hyd_pressure_psi;
  ElacComputer_Y.out.data.analog_inputs.yellow_hyd_pressure_psi = ElacComputer_B.yellow_hyd_pressure_psi;
  ElacComputer_Y.out.data.bus_inputs.adr_1_bus.altitude_standard_ft.SSM = ElacComputer_B.SSM_nl;
  ElacComputer_Y.out.data.bus_inputs.adr_1_bus.altitude_standard_ft.Data = ElacComputer_B.Data_ln;
  ElacComputer_Y.out.data.bus_inputs.adr_1_bus.altitude_corrected_ft.SSM = ElacComputer_B.SSM_n3;
  ElacComputer_Y.out.data.bus_inputs.adr_1_bus.altitude_corrected_ft.Data = ElacComputer_B.Data_fki;
  ElacComputer_Y.out.data.bus_inputs.adr_1_bus.mach.SSM = ElacComputer_B.SSM_a1;
  ElacComputer_Y.out.data.bus_inputs.adr_1_bus.mach.Data = ElacComputer_B.Data_bv;
  ElacComputer_Y.out.data.bus_inputs.adr_1_bus.airspeed_computed_kn.SSM = ElacComputer_B.SSM_p1;
  ElacComputer_Y.out.data.bus_inputs.adr_1_bus.airspeed_computed_kn.Data = ElacComputer_B.Data_m21;
  ElacComputer_Y.out.data.bus_inputs.adr_1_bus.airspeed_true_kn.SSM = ElacComputer_B.SSM_cn2;
  ElacComputer_Y.out.data.bus_inputs.adr_1_bus.airspeed_true_kn.Data = ElacComputer_B.Data_nbg;
  ElacComputer_Y.out.data.bus_inputs.adr_1_bus.vertical_speed_ft_min.SSM = ElacComputer_B.SSM_an3;
  ElacComputer_Y.out.data.bus_inputs.adr_1_bus.vertical_speed_ft_min.Data = ElacComputer_B.Data_l25;
  ElacComputer_Y.out.data.bus_inputs.adr_1_bus.aoa_corrected_deg.SSM = ElacComputer_B.SSM_c3;
  ElacComputer_Y.out.data.bus_inputs.adr_1_bus.aoa_corrected_deg.Data = ElacComputer_B.Data_ki;
  ElacComputer_Y.out.data.bus_inputs.adr_2_bus.altitude_standard_ft.SSM = ElacComputer_B.SSM_dp;
  ElacComputer_Y.out.data.bus_inputs.adr_2_bus.altitude_standard_ft.Data = ElacComputer_B.Data_p5p;
  ElacComputer_Y.out.data.bus_inputs.adr_2_bus.altitude_corrected_ft.SSM = ElacComputer_B.SSM_boy;
  ElacComputer_Y.out.data.bus_inputs.adr_2_bus.altitude_corrected_ft.Data = ElacComputer_B.Data_nry;
  ElacComputer_Y.out.data.bus_inputs.adr_2_bus.mach.SSM = ElacComputer_B.SSM_lg;
  ElacComputer_Y.out.data.bus_inputs.adr_2_bus.mach.Data = ElacComputer_B.Data_mh;
  ElacComputer_Y.out.data.bus_inputs.adr_2_bus.airspeed_computed_kn.SSM = ElacComputer_B.SSM_cm;
  ElacComputer_Y.out.data.bus_inputs.adr_2_bus.airspeed_computed_kn.Data = ElacComputer_B.Data_ll;
  ElacComputer_Y.out.data.bus_inputs.adr_2_bus.airspeed_true_kn.SSM = ElacComputer_B.SSM_hl;
  ElacComputer_Y.out.data.bus_inputs.adr_2_bus.airspeed_true_kn.Data = ElacComputer_B.Data_hy;
  ElacComputer_Y.out.data.bus_inputs.adr_2_bus.vertical_speed_ft_min.SSM = ElacComputer_B.SSM_irh;
  ElacComputer_Y.out.data.bus_inputs.adr_2_bus.vertical_speed_ft_min.Data = ElacComputer_B.Data_j04;
  ElacComputer_Y.out.data.bus_inputs.adr_2_bus.aoa_corrected_deg.SSM = ElacComputer_B.SSM_b42;
  ElacComputer_Y.out.data.bus_inputs.adr_2_bus.aoa_corrected_deg.Data = ElacComputer_B.Data_pf;
  ElacComputer_Y.out.data.bus_inputs.adr_3_bus.altitude_standard_ft.SSM = ElacComputer_B.SSM_anz;
  ElacComputer_Y.out.data.bus_inputs.adr_3_bus.altitude_standard_ft.Data = ElacComputer_B.Data_pl;
  ElacComputer_Y.out.data.bus_inputs.adr_3_bus.altitude_corrected_ft.SSM = ElacComputer_B.SSM_d2;
  ElacComputer_Y.out.data.bus_inputs.adr_3_bus.altitude_corrected_ft.Data = ElacComputer_B.Data_gb;
  ElacComputer_Y.out.data.bus_inputs.adr_3_bus.mach.SSM = ElacComputer_B.SSM_gov;
  ElacComputer_Y.out.data.bus_inputs.adr_3_bus.mach.Data = ElacComputer_B.Data_hq;
  ElacComputer_Y.out.data.bus_inputs.adr_3_bus.airspeed_computed_kn.SSM = ElacComputer_B.SSM_nb;
  ElacComputer_Y.out.data.bus_inputs.adr_3_bus.airspeed_computed_kn.Data = ElacComputer_B.Data_ai;
  ElacComputer_Y.out.data.bus_inputs.adr_3_bus.airspeed_true_kn.SSM = ElacComputer_B.SSM_pe3;
  ElacComputer_Y.out.data.bus_inputs.adr_3_bus.airspeed_true_kn.Data = ElacComputer_B.Data_gfr;
  ElacComputer_Y.out.data.bus_inputs.adr_3_bus.vertical_speed_ft_min.SSM = ElacComputer_B.SSM_jj;
  ElacComputer_Y.out.data.bus_inputs.adr_3_bus.vertical_speed_ft_min.Data = ElacComputer_B.Data_czp;
  ElacComputer_Y.out.data.bus_inputs.adr_3_bus.aoa_corrected_deg.SSM = ElacComputer_B.SSM_jx;
  ElacComputer_Y.out.data.bus_inputs.adr_3_bus.aoa_corrected_deg.Data = ElacComputer_B.Data_fm;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.discrete_word_1.SSM = ElacComputer_B.SSM_npl;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.discrete_word_1.Data = ElacComputer_B.Data_jsg;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.latitude_deg.SSM = ElacComputer_B.SSM;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.latitude_deg.Data = ElacComputer_B.Data;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.longitude_deg.SSM = ElacComputer_B.SSM_k;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.longitude_deg.Data = ElacComputer_B.Data_f;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.ground_speed_kn.SSM = ElacComputer_B.SSM_kx;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.ground_speed_kn.Data = ElacComputer_B.Data_fw;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.track_angle_true_deg.SSM = ElacComputer_B.SSM_kxx;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.track_angle_true_deg.Data = ElacComputer_B.Data_fwx;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.heading_true_deg.SSM = ElacComputer_B.SSM_kxxt;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.heading_true_deg.Data = ElacComputer_B.Data_fwxk;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.wind_speed_kn.SSM = ElacComputer_B.SSM_kxxta;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.wind_speed_kn.Data = ElacComputer_B.Data_fwxkf;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.wind_direction_true_deg.SSM = ElacComputer_B.SSM_kxxtac;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.wind_direction_true_deg.Data = ElacComputer_B.Data_fwxkft;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.track_angle_magnetic_deg.SSM = ElacComputer_B.SSM_kxxtac0;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.track_angle_magnetic_deg.Data = ElacComputer_B.Data_fwxkftc;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.heading_magnetic_deg.SSM = ElacComputer_B.SSM_kxxtac0z;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.heading_magnetic_deg.Data = ElacComputer_B.Data_fwxkftc3;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.drift_angle_deg.SSM = ElacComputer_B.SSM_kxxtac0zt;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.drift_angle_deg.Data = ElacComputer_B.Data_fwxkftc3e;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.flight_path_angle_deg.SSM = ElacComputer_B.SSM_kxxtac0ztg;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.flight_path_angle_deg.Data = ElacComputer_B.Data_fwxkftc3ep;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.flight_path_accel_g.SSM = ElacComputer_B.SSM_kxxtac0ztgf;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.flight_path_accel_g.Data = ElacComputer_B.Data_fwxkftc3epg;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.pitch_angle_deg.SSM = ElacComputer_B.SSM_kxxtac0ztgf2;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.pitch_angle_deg.Data = ElacComputer_B.Data_fwxkftc3epgt;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.roll_angle_deg.SSM = ElacComputer_B.SSM_kxxtac0ztgf2u;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.roll_angle_deg.Data = ElacComputer_B.Data_fwxkftc3epgtd;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.body_pitch_rate_deg_s.SSM = ElacComputer_B.SSM_kxxtac0ztgf2ux;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.body_pitch_rate_deg_s.Data = ElacComputer_B.Data_fwxkftc3epgtdx;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.body_roll_rate_deg_s.SSM = ElacComputer_B.SSM_kxxtac0ztgf2uxn;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.body_roll_rate_deg_s.Data = ElacComputer_B.Data_fwxkftc3epgtdxc;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.body_yaw_rate_deg_s.SSM = ElacComputer_B.SSM_ky;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.body_yaw_rate_deg_s.Data = ElacComputer_B.Data_h;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.body_long_accel_g.SSM = ElacComputer_B.SSM_d;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.body_long_accel_g.Data = ElacComputer_B.Data_e;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.body_lat_accel_g.SSM = ElacComputer_B.SSM_h;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.body_lat_accel_g.Data = ElacComputer_B.Data_j;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.body_normal_accel_g.SSM = ElacComputer_B.SSM_kb;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.body_normal_accel_g.Data = ElacComputer_B.Data_d;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.track_angle_rate_deg_s.SSM = ElacComputer_B.SSM_p;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.track_angle_rate_deg_s.Data = ElacComputer_B.Data_p;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.pitch_att_rate_deg_s.SSM = ElacComputer_B.SSM_di;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.pitch_att_rate_deg_s.Data = ElacComputer_B.Data_i;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.roll_att_rate_deg_s.SSM = ElacComputer_B.SSM_j;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.roll_att_rate_deg_s.Data = ElacComputer_B.Data_g;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.inertial_alt_ft.SSM = ElacComputer_B.SSM_i;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.inertial_alt_ft.Data = ElacComputer_B.Data_a;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.along_track_horiz_acc_g.SSM = ElacComputer_B.SSM_g;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.along_track_horiz_acc_g.Data = ElacComputer_B.Data_eb;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.cross_track_horiz_acc_g.SSM = ElacComputer_B.SSM_db;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.cross_track_horiz_acc_g.Data = ElacComputer_B.Data_jo;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.vertical_accel_g.SSM = ElacComputer_B.SSM_n;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.vertical_accel_g.Data = ElacComputer_B.Data_ex;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.inertial_vertical_speed_ft_s.SSM = ElacComputer_B.SSM_a;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.inertial_vertical_speed_ft_s.Data = ElacComputer_B.Data_fd;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.north_south_velocity_kn.SSM = ElacComputer_B.SSM_ir;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.north_south_velocity_kn.Data = ElacComputer_B.Data_ja;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.east_west_velocity_kn.SSM = ElacComputer_B.SSM_hu;
  ElacComputer_Y.out.data.bus_inputs.ir_1_bus.east_west_velocity_kn.Data = ElacComputer_B.Data_k;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.discrete_word_1.SSM = ElacComputer_B.SSM_e;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.discrete_word_1.Data = ElacComputer_B.Data_joy;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.latitude_deg.SSM = ElacComputer_B.SSM_gr;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.latitude_deg.Data = ElacComputer_B.Data_h3;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.longitude_deg.SSM = ElacComputer_B.SSM_ev;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.longitude_deg.Data = ElacComputer_B.Data_a0;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.ground_speed_kn.SSM = ElacComputer_B.SSM_l;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.ground_speed_kn.Data = ElacComputer_B.Data_b;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.track_angle_true_deg.SSM = ElacComputer_B.SSM_ei;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.track_angle_true_deg.Data = ElacComputer_B.Data_eq;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.heading_true_deg.SSM = ElacComputer_B.SSM_an;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.heading_true_deg.Data = ElacComputer_B.Data_iz;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.wind_speed_kn.SSM = ElacComputer_B.SSM_c;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.wind_speed_kn.Data = ElacComputer_B.Data_j2;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.wind_direction_true_deg.SSM = ElacComputer_B.SSM_cb;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.wind_direction_true_deg.Data = ElacComputer_B.Data_o;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.track_angle_magnetic_deg.SSM = ElacComputer_B.SSM_lb;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.track_angle_magnetic_deg.Data = ElacComputer_B.Data_m;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.heading_magnetic_deg.SSM = ElacComputer_B.SSM_ia;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.heading_magnetic_deg.Data = ElacComputer_B.Data_oq;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.drift_angle_deg.SSM = ElacComputer_B.SSM_kyz;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.drift_angle_deg.Data = ElacComputer_B.Data_fo;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.flight_path_angle_deg.SSM = ElacComputer_B.SSM_as;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.flight_path_angle_deg.Data = ElacComputer_B.Data_p1;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.flight_path_accel_g.SSM = ElacComputer_B.SSM_is;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.flight_path_accel_g.Data = ElacComputer_B.Data_p1y;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.pitch_angle_deg.SSM = ElacComputer_B.SSM_ca;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.pitch_angle_deg.Data = ElacComputer_B.Data_l;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.roll_angle_deg.SSM = ElacComputer_B.SSM_o;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.roll_angle_deg.Data = ElacComputer_B.Data_kp;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.body_pitch_rate_deg_s.SSM = ElacComputer_B.SSM_ak;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.body_pitch_rate_deg_s.Data = ElacComputer_B.Data_k0;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.body_roll_rate_deg_s.SSM = ElacComputer_B.SSM_cbj;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.body_roll_rate_deg_s.Data = ElacComputer_B.Data_pi;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.body_yaw_rate_deg_s.SSM = ElacComputer_B.SSM_cu;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.body_yaw_rate_deg_s.Data = ElacComputer_B.Data_dm;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.body_long_accel_g.SSM = ElacComputer_B.SSM_nn;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.body_long_accel_g.Data = ElacComputer_B.Data_f5;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.body_lat_accel_g.SSM = ElacComputer_B.SSM_b;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.body_lat_accel_g.Data = ElacComputer_B.Data_js;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.body_normal_accel_g.SSM = ElacComputer_B.SSM_m;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.body_normal_accel_g.Data = ElacComputer_B.Data_ee;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.track_angle_rate_deg_s.SSM = ElacComputer_B.SSM_f;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.track_angle_rate_deg_s.Data = ElacComputer_B.Data_ig;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.pitch_att_rate_deg_s.SSM = ElacComputer_B.SSM_bp;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.pitch_att_rate_deg_s.Data = ElacComputer_B.Data_mk;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.roll_att_rate_deg_s.SSM = ElacComputer_B.SSM_hb;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.roll_att_rate_deg_s.Data = ElacComputer_B.Data_pu;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.inertial_alt_ft.SSM = ElacComputer_B.SSM_gz;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.inertial_alt_ft.Data = ElacComputer_B.Data_ly;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.along_track_horiz_acc_g.SSM = ElacComputer_B.SSM_pv;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.along_track_horiz_acc_g.Data = ElacComputer_B.Data_jq;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.cross_track_horiz_acc_g.SSM = ElacComputer_B.SSM_mf;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.cross_track_horiz_acc_g.Data = ElacComputer_B.Data_o5;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.vertical_accel_g.SSM = ElacComputer_B.SSM_m0;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.vertical_accel_g.Data = ElacComputer_B.Data_lyw;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.inertial_vertical_speed_ft_s.SSM = ElacComputer_B.SSM_kd;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.inertial_vertical_speed_ft_s.Data = ElacComputer_B.Data_gq;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.north_south_velocity_kn.SSM = ElacComputer_B.SSM_pu;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.north_south_velocity_kn.Data = ElacComputer_B.Data_n;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.east_west_velocity_kn.SSM = ElacComputer_B.SSM_nv;
  ElacComputer_Y.out.data.bus_inputs.ir_2_bus.east_west_velocity_kn.Data = ElacComputer_B.Data_bq;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.discrete_word_1.SSM = ElacComputer_B.SSM_d5;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.discrete_word_1.Data = ElacComputer_B.Data_dmn;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.latitude_deg.SSM = ElacComputer_B.SSM_eo;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.latitude_deg.Data = ElacComputer_B.Data_jn;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.longitude_deg.SSM = ElacComputer_B.SSM_nd;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.longitude_deg.Data = ElacComputer_B.Data_c;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.ground_speed_kn.SSM = ElacComputer_B.SSM_bq;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.ground_speed_kn.Data = ElacComputer_B.Data_lx;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.track_angle_true_deg.SSM = ElacComputer_B.SSM_hi;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.track_angle_true_deg.Data = ElacComputer_B.Data_jb;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.heading_true_deg.SSM = ElacComputer_B.SSM_mm;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.heading_true_deg.Data = ElacComputer_B.Data_fn;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.wind_speed_kn.SSM = ElacComputer_B.SSM_kz;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.wind_speed_kn.Data = ElacComputer_B.Data_od;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.wind_direction_true_deg.SSM = ElacComputer_B.SSM_il;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.wind_direction_true_deg.Data = ElacComputer_B.Data_ez;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.track_angle_magnetic_deg.SSM = ElacComputer_B.SSM_i2;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.track_angle_magnetic_deg.Data = ElacComputer_B.Data_pw;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.heading_magnetic_deg.SSM = ElacComputer_B.SSM_ah;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.heading_magnetic_deg.Data = ElacComputer_B.Data_m2;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.drift_angle_deg.SSM = ElacComputer_B.SSM_en;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.drift_angle_deg.Data = ElacComputer_B.Data_ek;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.flight_path_angle_deg.SSM = ElacComputer_B.SSM_dq;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.flight_path_angle_deg.Data = ElacComputer_B.Data_iy;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.flight_path_accel_g.SSM = ElacComputer_B.SSM_px;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.flight_path_accel_g.Data = ElacComputer_B.Data_lk;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.pitch_angle_deg.SSM = ElacComputer_B.SSM_lbo;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.pitch_angle_deg.Data = ElacComputer_B.Data_ca;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.roll_angle_deg.SSM = ElacComputer_B.SSM_p5;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.roll_angle_deg.Data = ElacComputer_B.Data_pix;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.body_pitch_rate_deg_s.SSM = ElacComputer_B.SSM_mk;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.body_pitch_rate_deg_s.Data = ElacComputer_B.Data_di;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.body_roll_rate_deg_s.SSM = ElacComputer_B.SSM_mu;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.body_roll_rate_deg_s.Data = ElacComputer_B.Data_lz;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.body_yaw_rate_deg_s.SSM = ElacComputer_B.SSM_cbl;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.body_yaw_rate_deg_s.Data = ElacComputer_B.Data_lu;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.body_long_accel_g.SSM = ElacComputer_B.SSM_gzd;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.body_long_accel_g.Data = ElacComputer_B.Data_dc;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.body_lat_accel_g.SSM = ElacComputer_B.SSM_mo;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.body_lat_accel_g.Data = ElacComputer_B.Data_gc;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.body_normal_accel_g.SSM = ElacComputer_B.SSM_me;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.body_normal_accel_g.Data = ElacComputer_B.Data_am;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.track_angle_rate_deg_s.SSM = ElacComputer_B.SSM_mj;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.track_angle_rate_deg_s.Data = ElacComputer_B.Data_mo;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.pitch_att_rate_deg_s.SSM = ElacComputer_B.SSM_a5;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.pitch_att_rate_deg_s.Data = ElacComputer_B.Data_dg;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.roll_att_rate_deg_s.SSM = ElacComputer_B.SSM_bt;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.roll_att_rate_deg_s.Data = ElacComputer_B.Data_e1;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.inertial_alt_ft.SSM = ElacComputer_B.SSM_om;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.inertial_alt_ft.Data = ElacComputer_B.Data_fp;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.along_track_horiz_acc_g.SSM = ElacComputer_B.SSM_ar;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.along_track_horiz_acc_g.Data = ElacComputer_B.Data_ns;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.cross_track_horiz_acc_g.SSM = ElacComputer_B.SSM_ce;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.cross_track_horiz_acc_g.Data = ElacComputer_B.Data_m3;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.vertical_accel_g.SSM = ElacComputer_B.SSM_ed;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.vertical_accel_g.Data = ElacComputer_B.Data_oj;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.inertial_vertical_speed_ft_s.SSM = ElacComputer_B.SSM_jh;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.inertial_vertical_speed_ft_s.Data = ElacComputer_B.Data_jy;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.north_south_velocity_kn.SSM = ElacComputer_B.SSM_je;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.north_south_velocity_kn.Data = ElacComputer_B.Data_j1;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.east_west_velocity_kn.SSM = ElacComputer_B.SSM_jt;
  ElacComputer_Y.out.data.bus_inputs.ir_3_bus.east_west_velocity_kn.Data = ElacComputer_B.Data_fc;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.fac_weight_lbs.SSM = ElacComputer_B.SSM_cui;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.fac_weight_lbs.Data = ElacComputer_B.Data_of;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.fm_weight_lbs.SSM = ElacComputer_B.SSM_mq;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.fm_weight_lbs.Data = ElacComputer_B.Data_lg;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.fac_cg_percent.SSM = ElacComputer_B.SSM_ni;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.fac_cg_percent.Data = ElacComputer_B.Data_n4;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.fm_cg_percent.SSM = ElacComputer_B.SSM_df;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.fm_cg_percent.Data = ElacComputer_B.Data_ot;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.fg_radio_height_ft.SSM = ElacComputer_B.SSM_oe;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.fg_radio_height_ft.Data = ElacComputer_B.Data_gv;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.discrete_word_4.SSM = ElacComputer_B.SSM_ha;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.discrete_word_4.Data = ElacComputer_B.Data_ou;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.ats_discrete_word.SSM = ElacComputer_B.SSM_op;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.ats_discrete_word.Data = ElacComputer_B.Data_dh;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.discrete_word_3.SSM = ElacComputer_B.SSM_a50;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.discrete_word_3.Data = ElacComputer_B.Data_ph;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.discrete_word_1.SSM = ElacComputer_B.SSM_og;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.discrete_word_1.Data = ElacComputer_B.Data_gs;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.discrete_word_2.SSM = ElacComputer_B.SSM_a4;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.discrete_word_2.Data = ElacComputer_B.Data_fd4;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.approach_spd_target_kn.SSM = ElacComputer_B.SSM_bv;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.approach_spd_target_kn.Data = ElacComputer_B.Data_hm;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.delta_p_ail_cmd_deg.SSM = ElacComputer_B.SSM_bo;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.delta_p_ail_cmd_deg.Data = ElacComputer_B.Data_i2;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.delta_p_splr_cmd_deg.SSM = ElacComputer_B.SSM_d1;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.delta_p_splr_cmd_deg.Data = ElacComputer_B.Data_og;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.delta_r_cmd_deg.SSM = ElacComputer_B.SSM_hy;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.delta_r_cmd_deg.Data = ElacComputer_B.Data_fv;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.delta_nose_wheel_cmd_deg.SSM = ElacComputer_B.SSM_gi;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.delta_nose_wheel_cmd_deg.Data = ElacComputer_B.Data_oc;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.delta_q_cmd_deg.SSM = ElacComputer_B.SSM_pp;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.delta_q_cmd_deg.Data = ElacComputer_B.Data_kq;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.n1_left_percent.SSM = ElacComputer_B.SSM_iab;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.n1_left_percent.Data = ElacComputer_B.Data_ne;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.n1_right_percent.SSM = ElacComputer_B.SSM_jtv;
  ElacComputer_Y.out.data.bus_inputs.fmgc_1_bus.n1_right_percent.Data = ElacComputer_B.Data_it;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.fac_weight_lbs.SSM = ElacComputer_B.SSM_fy;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.fac_weight_lbs.Data = ElacComputer_B.Data_ch;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.fm_weight_lbs.SSM = ElacComputer_B.SSM_d4;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.fm_weight_lbs.Data = ElacComputer_B.Data_bb;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.fac_cg_percent.SSM = ElacComputer_B.SSM_ars;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.fac_cg_percent.Data = ElacComputer_B.Data_ol;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.fm_cg_percent.SSM = ElacComputer_B.SSM_din;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.fm_cg_percent.Data = ElacComputer_B.Data_hw;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.fg_radio_height_ft.SSM = ElacComputer_B.SSM_m3;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.fg_radio_height_ft.Data = ElacComputer_B.Data_hs;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.discrete_word_4.SSM = ElacComputer_B.SSM_np;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.discrete_word_4.Data = ElacComputer_B.Data_fj;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.ats_discrete_word.SSM = ElacComputer_B.SSM_ax;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.ats_discrete_word.Data = ElacComputer_B.Data_ky;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.discrete_word_3.SSM = ElacComputer_B.SSM_cl;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.discrete_word_3.Data = ElacComputer_B.Data_h5;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.discrete_word_1.SSM = ElacComputer_B.SSM_es;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.discrete_word_1.Data = ElacComputer_B.Data_ku;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.discrete_word_2.SSM = ElacComputer_B.SSM_gi1;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.discrete_word_2.Data = ElacComputer_B.Data_jp;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.approach_spd_target_kn.SSM = ElacComputer_B.SSM_jz;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.approach_spd_target_kn.Data = ElacComputer_B.Data_nu;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.delta_p_ail_cmd_deg.SSM = ElacComputer_B.SSM_kt;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.delta_p_ail_cmd_deg.Data = ElacComputer_B.Data_br;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.delta_p_splr_cmd_deg.SSM = ElacComputer_B.SSM_ds;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.delta_p_splr_cmd_deg.Data = ElacComputer_B.Data_ae;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.delta_r_cmd_deg.SSM = ElacComputer_B.SSM_eg;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.delta_r_cmd_deg.Data = ElacComputer_B.Data_pe;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.delta_nose_wheel_cmd_deg.SSM = ElacComputer_B.SSM_a0;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.delta_nose_wheel_cmd_deg.Data = ElacComputer_B.Data_fy;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.delta_q_cmd_deg.SSM = ElacComputer_B.SSM_cv;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.delta_q_cmd_deg.Data = ElacComputer_B.Data_na;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.n1_left_percent.SSM = ElacComputer_B.SSM_ea;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.n1_left_percent.Data = ElacComputer_B.Data_my;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.n1_right_percent.SSM = ElacComputer_B.SSM_p4;
  ElacComputer_Y.out.data.bus_inputs.fmgc_2_bus.n1_right_percent.Data = ElacComputer_B.Data_i4;
  ElacComputer_Y.out.data.bus_inputs.ra_1_bus.radio_height_ft.SSM = ElacComputer_B.SSM_m2;
  ElacComputer_Y.out.data.bus_inputs.ra_1_bus.radio_height_ft.Data = ElacComputer_B.Data_cx;
  ElacComputer_Y.out.data.bus_inputs.ra_2_bus.radio_height_ft.SSM = ElacComputer_B.SSM_bt0;
  ElacComputer_Y.out.data.bus_inputs.ra_2_bus.radio_height_ft.Data = ElacComputer_B.Data_nz;
  ElacComputer_Y.out.data.bus_inputs.sfcc_1_bus.slat_flap_component_status_word.SSM = ElacComputer_B.SSM_nr;
  ElacComputer_Y.out.data.bus_inputs.sfcc_1_bus.slat_flap_component_status_word.Data = ElacComputer_B.Data_id;
  ElacComputer_Y.out.data.bus_inputs.sfcc_1_bus.slat_flap_system_status_word.SSM = ElacComputer_B.SSM_fr;
  ElacComputer_Y.out.data.bus_inputs.sfcc_1_bus.slat_flap_system_status_word.Data = ElacComputer_B.Data_o2;
  ElacComputer_Y.out.data.bus_inputs.sfcc_1_bus.slat_flap_actual_position_word.SSM = ElacComputer_B.SSM_cc;
  ElacComputer_Y.out.data.bus_inputs.sfcc_1_bus.slat_flap_actual_position_word.Data = ElacComputer_B.Data_gqq;
  ElacComputer_Y.out.data.bus_inputs.sfcc_1_bus.slat_actual_position_deg.SSM = ElacComputer_B.SSM_lm;
  ElacComputer_Y.out.data.bus_inputs.sfcc_1_bus.slat_actual_position_deg.Data = ElacComputer_B.Data_md;
  ElacComputer_Y.out.data.bus_inputs.sfcc_1_bus.flap_actual_position_deg.SSM = ElacComputer_B.SSM_mkm;
  ElacComputer_Y.out.data.bus_inputs.sfcc_1_bus.flap_actual_position_deg.Data = ElacComputer_B.Data_cz;
  ElacComputer_Y.out.data.bus_inputs.sfcc_2_bus.slat_flap_component_status_word.SSM = ElacComputer_B.SSM_jhd;
  ElacComputer_Y.out.data.bus_inputs.sfcc_2_bus.slat_flap_component_status_word.Data = ElacComputer_B.Data_pm;
  ElacComputer_Y.out.data.bus_inputs.sfcc_2_bus.slat_flap_system_status_word.SSM = ElacComputer_B.SSM_av;
  ElacComputer_Y.out.data.bus_inputs.sfcc_2_bus.slat_flap_system_status_word.Data = ElacComputer_B.Data_bj;
  ElacComputer_Y.out.data.bus_inputs.sfcc_2_bus.slat_flap_actual_position_word.SSM = ElacComputer_B.SSM_ira;
  ElacComputer_Y.out.data.bus_inputs.sfcc_2_bus.slat_flap_actual_position_word.Data = ElacComputer_B.Data_ox;
  ElacComputer_Y.out.data.bus_inputs.sfcc_2_bus.slat_actual_position_deg.SSM = ElacComputer_B.SSM_ge;
  ElacComputer_Y.out.data.bus_inputs.sfcc_2_bus.slat_actual_position_deg.Data = ElacComputer_B.Data_pe5;
  ElacComputer_Y.out.data.bus_inputs.sfcc_2_bus.flap_actual_position_deg.SSM = ElacComputer_B.SSM_lv;
  ElacComputer_Y.out.data.bus_inputs.sfcc_2_bus.flap_actual_position_deg.Data = ElacComputer_B.Data_jj;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.efcs_status_word_1.SSM = ElacComputer_B.SSM_cg;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.efcs_status_word_1.Data = ElacComputer_B.Data_p5;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.efcs_status_word_2.SSM = ElacComputer_B.SSM_be;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.efcs_status_word_2.Data = ElacComputer_B.Data_ekl;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.efcs_status_word_3.SSM = ElacComputer_B.SSM_axb;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.efcs_status_word_3.Data = ElacComputer_B.Data_nd;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.efcs_status_word_4.SSM = ElacComputer_B.SSM_nz;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.efcs_status_word_4.Data = ElacComputer_B.Data_n2;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.efcs_status_word_5.SSM = ElacComputer_B.SSM_cx;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.efcs_status_word_5.Data = ElacComputer_B.Data_dl;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.capt_roll_command_deg.SSM = ElacComputer_B.SSM_gh;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.capt_roll_command_deg.Data = ElacComputer_B.Data_gs2;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.fo_roll_command_deg.SSM = ElacComputer_B.SSM_ks;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.fo_roll_command_deg.Data = ElacComputer_B.Data_h4;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.rudder_pedal_position_deg.SSM = ElacComputer_B.SSM_pw;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.rudder_pedal_position_deg.Data = ElacComputer_B.Data_e3;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.capt_pitch_command_deg.SSM = ElacComputer_B.SSM_fh;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.capt_pitch_command_deg.Data = ElacComputer_B.Data_f5h;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.fo_pitch_command_deg.SSM = ElacComputer_B.SSM_gzn;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.fo_pitch_command_deg.Data = ElacComputer_B.Data_an;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.aileron_left_pos_deg.SSM = ElacComputer_B.SSM_oo;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.aileron_left_pos_deg.Data = ElacComputer_B.Data_i4o;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.elevator_left_pos_deg.SSM = ElacComputer_B.SSM_evh;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.elevator_left_pos_deg.Data = ElacComputer_B.Data_af;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.aileron_right_pos_deg.SSM = ElacComputer_B.SSM_cn;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.aileron_right_pos_deg.Data = ElacComputer_B.Data_bm;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.elevator_right_pos_deg.SSM = ElacComputer_B.SSM_co;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.elevator_right_pos_deg.Data = ElacComputer_B.Data_dk;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.horiz_stab_trim_pos_deg.SSM = ElacComputer_B.SSM_pe;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.horiz_stab_trim_pos_deg.Data = ElacComputer_B.Data_nv;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_1_left_pos_deg.SSM = ElacComputer_B.SSM_cgz;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_1_left_pos_deg.Data = ElacComputer_B.Data_jpf;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_2_left_pos_deg.SSM = ElacComputer_B.SSM_fw;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_2_left_pos_deg.Data = ElacComputer_B.Data_i5;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_3_left_pos_deg.SSM = ElacComputer_B.SSM_h4;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_3_left_pos_deg.Data = ElacComputer_B.Data_k2;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_4_left_pos_deg.SSM = ElacComputer_B.SSM_cb3;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_4_left_pos_deg.Data = ElacComputer_B.Data_as;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_5_left_pos_deg.SSM = ElacComputer_B.SSM_pj;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_5_left_pos_deg.Data = ElacComputer_B.Data_gk;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_1_right_pos_deg.SSM = ElacComputer_B.SSM_dv;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_1_right_pos_deg.Data = ElacComputer_B.Data_jl;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_2_right_pos_deg.SSM = ElacComputer_B.SSM_i4;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_2_right_pos_deg.Data = ElacComputer_B.Data_e32;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_3_right_pos_deg.SSM = ElacComputer_B.SSM_fm;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_3_right_pos_deg.Data = ElacComputer_B.Data_ih;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_4_right_pos_deg.SSM = ElacComputer_B.SSM_e5;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_4_right_pos_deg.Data = ElacComputer_B.Data_du;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_5_right_pos_deg.SSM = ElacComputer_B.SSM_bf;
  ElacComputer_Y.out.data.bus_inputs.fcdc_1_bus.spoiler_5_right_pos_deg.Data = ElacComputer_B.Data_nx;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.efcs_status_word_1.SSM = ElacComputer_B.SSM_fd;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.efcs_status_word_1.Data = ElacComputer_B.Data_n0;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.efcs_status_word_2.SSM = ElacComputer_B.SSM_fv;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.efcs_status_word_2.Data = ElacComputer_B.Data_eqi;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.efcs_status_word_3.SSM = ElacComputer_B.SSM_dt;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.efcs_status_word_3.Data = ElacComputer_B.Data_om;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.efcs_status_word_4.SSM = ElacComputer_B.SSM_j5;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.efcs_status_word_4.Data = ElacComputer_B.Data_nr;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.efcs_status_word_5.SSM = ElacComputer_B.SSM_ng;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.efcs_status_word_5.Data = ElacComputer_B.Data_p3;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.capt_roll_command_deg.SSM = ElacComputer_B.SSM_cs;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.capt_roll_command_deg.Data = ElacComputer_B.Data_nb;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.fo_roll_command_deg.SSM = ElacComputer_B.SSM_ls;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.fo_roll_command_deg.Data = ElacComputer_B.Data_hd;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.rudder_pedal_position_deg.SSM = ElacComputer_B.SSM_dg;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.rudder_pedal_position_deg.Data = ElacComputer_B.Data_al;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.capt_pitch_command_deg.SSM = ElacComputer_B.SSM_d3;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.capt_pitch_command_deg.Data = ElacComputer_B.Data_gu;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.fo_pitch_command_deg.SSM = ElacComputer_B.SSM_p2;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.fo_pitch_command_deg.Data = ElacComputer_B.Data_ix;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.aileron_left_pos_deg.SSM = ElacComputer_B.SSM_bo0;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.aileron_left_pos_deg.Data = ElacComputer_B.Data_do;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.elevator_left_pos_deg.SSM = ElacComputer_B.SSM_bc;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.elevator_left_pos_deg.Data = ElacComputer_B.Data_hu;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.aileron_right_pos_deg.SSM = ElacComputer_B.SSM_h0;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.aileron_right_pos_deg.Data = ElacComputer_B.Data_pm1;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.elevator_right_pos_deg.SSM = ElacComputer_B.SSM_giz;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.elevator_right_pos_deg.Data = ElacComputer_B.Data_i2y;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.horiz_stab_trim_pos_deg.SSM = ElacComputer_B.SSM_mqp;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.horiz_stab_trim_pos_deg.Data = ElacComputer_B.Data_pg;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_1_left_pos_deg.SSM = ElacComputer_B.SSM_ba;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_1_left_pos_deg.Data = ElacComputer_B.Data_ni;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_2_left_pos_deg.SSM = ElacComputer_B.SSM_in;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_2_left_pos_deg.Data = ElacComputer_B.Data_fr;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_3_left_pos_deg.SSM = ElacComputer_B.SSM_ff;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_3_left_pos_deg.Data = ElacComputer_B.Data_cn;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_4_left_pos_deg.SSM = ElacComputer_B.SSM_ic;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_4_left_pos_deg.Data = ElacComputer_B.Data_nxl;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_5_left_pos_deg.SSM = ElacComputer_B.SSM_fs;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_5_left_pos_deg.Data = ElacComputer_B.Data_jh;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_1_right_pos_deg.SSM = ElacComputer_B.SSM_ja;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_1_right_pos_deg.Data = ElacComputer_B.Data_gl;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_2_right_pos_deg.SSM = ElacComputer_B.SSM_js;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_2_right_pos_deg.Data = ElacComputer_B.Data_gn;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_3_right_pos_deg.SSM = ElacComputer_B.SSM_is3;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_3_right_pos_deg.Data = ElacComputer_B.Data_myb;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_4_right_pos_deg.SSM = ElacComputer_B.SSM_ag;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_4_right_pos_deg.Data = ElacComputer_B.Data_l2;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_5_right_pos_deg.SSM = ElacComputer_B.SSM_f5;
  ElacComputer_Y.out.data.bus_inputs.fcdc_2_bus.spoiler_5_right_pos_deg.Data = ElacComputer_B.Data_o5o;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.left_spoiler_1_position_deg.SSM = ElacComputer_B.SSM_ph;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.left_spoiler_1_position_deg.Data = ElacComputer_B.Data_l5;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.right_spoiler_1_position_deg.SSM = ElacComputer_B.SSM_jw;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.right_spoiler_1_position_deg.Data = ElacComputer_B.Data_dc2;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.left_spoiler_2_position_deg.SSM = ElacComputer_B.SSM_jy;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.left_spoiler_2_position_deg.Data = ElacComputer_B.Data_gr;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.right_spoiler_2_position_deg.SSM = ElacComputer_B.SSM_j1;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.right_spoiler_2_position_deg.Data = ElacComputer_B.Data_gp;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.left_elevator_position_deg.SSM = ElacComputer_B.SSM_ov;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.left_elevator_position_deg.Data = ElacComputer_B.Data_i3;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.right_elevator_position_deg.SSM = ElacComputer_B.SSM_mx;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.right_elevator_position_deg.Data = ElacComputer_B.Data_et;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.ths_position_deg.SSM = ElacComputer_B.SSM_b4;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.ths_position_deg.Data = ElacComputer_B.Data_mc;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.left_sidestick_pitch_command_deg.SSM = ElacComputer_B.SSM_gb;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.left_sidestick_pitch_command_deg.Data = ElacComputer_B.Data_k3;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.right_sidestick_pitch_command_deg.SSM = ElacComputer_B.SSM_oh;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.right_sidestick_pitch_command_deg.Data = ElacComputer_B.Data_f2;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.left_sidestick_roll_command_deg.SSM = ElacComputer_B.SSM_mm5;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.left_sidestick_roll_command_deg.Data = ElacComputer_B.Data_gh;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.right_sidestick_roll_command_deg.SSM = ElacComputer_B.SSM_br;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.right_sidestick_roll_command_deg.Data = ElacComputer_B.Data_ed;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.speed_brake_lever_command_deg.SSM = ElacComputer_B.SSM_c2;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.speed_brake_lever_command_deg.Data = ElacComputer_B.Data_o2j;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.thrust_lever_angle_1_deg.SSM = ElacComputer_B.SSM_hc;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.thrust_lever_angle_1_deg.Data = ElacComputer_B.Data_i43;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.thrust_lever_angle_2_deg.SSM = ElacComputer_B.SSM_ktr;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.thrust_lever_angle_2_deg.Data = ElacComputer_B.Data_ic;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.discrete_status_word_1.SSM = ElacComputer_B.SSM_gl;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.discrete_status_word_1.Data = ElacComputer_B.Data_ak;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.discrete_status_word_2.SSM = ElacComputer_B.SSM_my;
  ElacComputer_Y.out.data.bus_inputs.sec_1_bus.discrete_status_word_2.Data = ElacComputer_B.Data_jg;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.left_spoiler_1_position_deg.SSM = ElacComputer_B.SSM_j3;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.left_spoiler_1_position_deg.Data = ElacComputer_B.Data_cu;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.right_spoiler_1_position_deg.SSM = ElacComputer_B.SSM_go;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.right_spoiler_1_position_deg.Data = ElacComputer_B.Data_ep;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.left_spoiler_2_position_deg.SSM = ElacComputer_B.SSM_e5c;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.left_spoiler_2_position_deg.Data = ElacComputer_B.Data_d3;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.right_spoiler_2_position_deg.SSM = ElacComputer_B.SSM_dk;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.right_spoiler_2_position_deg.Data = ElacComputer_B.Data_bt;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.left_elevator_position_deg.SSM = ElacComputer_B.SSM_evc;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.left_elevator_position_deg.Data = ElacComputer_B.Data_e0;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.right_elevator_position_deg.SSM = ElacComputer_B.SSM_kk;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.right_elevator_position_deg.Data = ElacComputer_B.Data_jl3;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.ths_position_deg.SSM = ElacComputer_B.SSM_af;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.ths_position_deg.Data = ElacComputer_B.Data_nm;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.left_sidestick_pitch_command_deg.SSM = ElacComputer_B.SSM_npr;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.left_sidestick_pitch_command_deg.Data = ElacComputer_B.Data_ia;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.right_sidestick_pitch_command_deg.SSM = ElacComputer_B.SSM_ew;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.right_sidestick_pitch_command_deg.Data = ElacComputer_B.Data_j0;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.left_sidestick_roll_command_deg.SSM = ElacComputer_B.SSM_lt;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.left_sidestick_roll_command_deg.Data = ElacComputer_B.Data_bs;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.right_sidestick_roll_command_deg.SSM = ElacComputer_B.SSM_ger;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.right_sidestick_roll_command_deg.Data = ElacComputer_B.Data_hp;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.speed_brake_lever_command_deg.SSM = ElacComputer_B.SSM_pxo;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.speed_brake_lever_command_deg.Data = ElacComputer_B.Data_ct;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.thrust_lever_angle_1_deg.SSM = ElacComputer_B.SSM_co2;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.thrust_lever_angle_1_deg.Data = ElacComputer_B.Data_pc;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.thrust_lever_angle_2_deg.SSM = ElacComputer_B.SSM_ny;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.thrust_lever_angle_2_deg.Data = ElacComputer_B.Data_nzt;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.discrete_status_word_1.SSM = ElacComputer_B.SSM_l4;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.discrete_status_word_1.Data = ElacComputer_B.Data_c0;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.discrete_status_word_2.SSM = ElacComputer_B.SSM_nm;
  ElacComputer_Y.out.data.bus_inputs.sec_2_bus.discrete_status_word_2.Data = ElacComputer_B.Data_ojg;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.left_aileron_position_deg.SSM = ElacComputer_B.SSM_nh;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.left_aileron_position_deg.Data = ElacComputer_B.Data_lm;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.right_aileron_position_deg.SSM = ElacComputer_B.SSM_dl;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.right_aileron_position_deg.Data = ElacComputer_B.Data_fz;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.left_elevator_position_deg.SSM = ElacComputer_B.SSM_dx;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.left_elevator_position_deg.Data = ElacComputer_B.Data_oz;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.right_elevator_position_deg.SSM = ElacComputer_B.SSM_a5h;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.right_elevator_position_deg.Data = ElacComputer_B.Data_gf;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.ths_position_deg.SSM = ElacComputer_B.SSM_fl;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.ths_position_deg.Data = ElacComputer_B.Data_nn;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.left_sidestick_pitch_command_deg.SSM = ElacComputer_B.SSM_p3;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.left_sidestick_pitch_command_deg.Data = ElacComputer_B.Data_a0z;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.right_sidestick_pitch_command_deg.SSM = ElacComputer_B.SSM_ns;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.right_sidestick_pitch_command_deg.Data = ElacComputer_B.Data_fk;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.left_sidestick_roll_command_deg.SSM = ElacComputer_B.SSM_bm;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.left_sidestick_roll_command_deg.Data = ElacComputer_B.Data_bu;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.right_sidestick_roll_command_deg.SSM = ElacComputer_B.SSM_grm;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.right_sidestick_roll_command_deg.Data = ElacComputer_B.Data_o23;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.rudder_pedal_position_deg.SSM = ElacComputer_B.SSM_gzm;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.rudder_pedal_position_deg.Data = ElacComputer_B.Data_g3;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.aileron_command_deg.SSM = ElacComputer_B.SSM_oi;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.aileron_command_deg.Data = ElacComputer_B.Data_icc;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.roll_spoiler_command_deg.SSM = ElacComputer_B.SSM_aa;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.roll_spoiler_command_deg.Data = ElacComputer_B.Data_pwf;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.yaw_damper_command_deg.SSM = ElacComputer_B.SSM_fvk;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.yaw_damper_command_deg.Data = ElacComputer_B.Data_gvk;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.elevator_double_pressurization_command_deg.SSM = ElacComputer_B.SSM_lw;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.elevator_double_pressurization_command_deg.Data =
    ElacComputer_B.Data_ka;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.discrete_status_word_1.SSM = ElacComputer_B.SSM_fa;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.discrete_status_word_1.Data = ElacComputer_B.Data_mp;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.discrete_status_word_2.SSM = ElacComputer_B.SSM_lbx;
  ElacComputer_Y.out.data.bus_inputs.elac_opp_bus.discrete_status_word_2.Data = ElacComputer_B.Data_m4;
  ElacComputer_Y.out.laws = ElacComputer_B.laws;
  ElacComputer_Y.out.logic = ElacComputer_B.logic;
}

void ElacComputer::initialize()
{
  ElacComputer_DWork.Delay_DSTATE_cc = ElacComputer_P.Delay_InitialCondition_c;
  ElacComputer_DWork.Delay1_DSTATE = ElacComputer_P.Delay1_InitialCondition;
  ElacComputer_DWork.Memory_PreviousInput = ElacComputer_P.SRFlipFlop_initial_condition;
  ElacComputer_DWork.Delay_DSTATE = ElacComputer_P.DiscreteDerivativeVariableTs_InitialCondition;
  ElacComputer_DWork.Delay_DSTATE_b = ElacComputer_P.Delay_InitialCondition;
  ElacComputer_DWork.icLoad = true;
  LawMDLOBJ2.init();
  LawMDLOBJ5.init();
  LawMDLOBJ3.init();
  ElacComputer_B.dt = ElacComputer_P.out_Y0.data.time.dt;
  ElacComputer_B.simulation_time = ElacComputer_P.out_Y0.data.time.simulation_time;
  ElacComputer_B.monotonic_time = ElacComputer_P.out_Y0.data.time.monotonic_time;
  ElacComputer_B.slew_on = ElacComputer_P.out_Y0.data.sim_data.slew_on;
  ElacComputer_B.pause_on = ElacComputer_P.out_Y0.data.sim_data.pause_on;
  ElacComputer_B.tracking_mode_on_override = ElacComputer_P.out_Y0.data.sim_data.tracking_mode_on_override;
  ElacComputer_B.tailstrike_protection_on = ElacComputer_P.out_Y0.data.sim_data.tailstrike_protection_on;
  ElacComputer_B.computer_running = ElacComputer_P.out_Y0.data.sim_data.computer_running;
  ElacComputer_B.ground_spoilers_active_1 = ElacComputer_P.out_Y0.data.discrete_inputs.ground_spoilers_active_1;
  ElacComputer_B.ground_spoilers_active_2 = ElacComputer_P.out_Y0.data.discrete_inputs.ground_spoilers_active_2;
  ElacComputer_B.is_unit_1 = ElacComputer_P.out_Y0.data.discrete_inputs.is_unit_1;
  ElacComputer_B.is_unit_2 = ElacComputer_P.out_Y0.data.discrete_inputs.is_unit_2;
  ElacComputer_B.opp_axis_pitch_failure = ElacComputer_P.out_Y0.data.discrete_inputs.opp_axis_pitch_failure;
  ElacComputer_B.ap_1_disengaged = ElacComputer_P.out_Y0.data.discrete_inputs.ap_1_disengaged;
  ElacComputer_B.ap_2_disengaged = ElacComputer_P.out_Y0.data.discrete_inputs.ap_2_disengaged;
  ElacComputer_B.opp_left_aileron_lost = ElacComputer_P.out_Y0.data.discrete_inputs.opp_left_aileron_lost;
  ElacComputer_B.opp_right_aileron_lost = ElacComputer_P.out_Y0.data.discrete_inputs.opp_right_aileron_lost;
  ElacComputer_B.fac_1_yaw_control_lost = ElacComputer_P.out_Y0.data.discrete_inputs.fac_1_yaw_control_lost;
  ElacComputer_B.lgciu_1_nose_gear_pressed = ElacComputer_P.out_Y0.data.discrete_inputs.lgciu_1_nose_gear_pressed;
  ElacComputer_B.lgciu_2_nose_gear_pressed = ElacComputer_P.out_Y0.data.discrete_inputs.lgciu_2_nose_gear_pressed;
  ElacComputer_B.fac_2_yaw_control_lost = ElacComputer_P.out_Y0.data.discrete_inputs.fac_2_yaw_control_lost;
  ElacComputer_B.lgciu_1_right_main_gear_pressed =
    ElacComputer_P.out_Y0.data.discrete_inputs.lgciu_1_right_main_gear_pressed;
  ElacComputer_B.lgciu_2_right_main_gear_pressed =
    ElacComputer_P.out_Y0.data.discrete_inputs.lgciu_2_right_main_gear_pressed;
  ElacComputer_B.lgciu_1_left_main_gear_pressed =
    ElacComputer_P.out_Y0.data.discrete_inputs.lgciu_1_left_main_gear_pressed;
  ElacComputer_B.lgciu_2_left_main_gear_pressed =
    ElacComputer_P.out_Y0.data.discrete_inputs.lgciu_2_left_main_gear_pressed;
  ElacComputer_B.ths_motor_fault = ElacComputer_P.out_Y0.data.discrete_inputs.ths_motor_fault;
  ElacComputer_B.sfcc_1_slats_out = ElacComputer_P.out_Y0.data.discrete_inputs.sfcc_1_slats_out;
  ElacComputer_B.sfcc_2_slats_out = ElacComputer_P.out_Y0.data.discrete_inputs.sfcc_2_slats_out;
  ElacComputer_B.l_ail_servo_failed = ElacComputer_P.out_Y0.data.discrete_inputs.l_ail_servo_failed;
  ElacComputer_B.l_elev_servo_failed = ElacComputer_P.out_Y0.data.discrete_inputs.l_elev_servo_failed;
  ElacComputer_B.r_ail_servo_failed = ElacComputer_P.out_Y0.data.discrete_inputs.r_ail_servo_failed;
  ElacComputer_B.r_elev_servo_failed = ElacComputer_P.out_Y0.data.discrete_inputs.r_elev_servo_failed;
  ElacComputer_B.ths_override_active = ElacComputer_P.out_Y0.data.discrete_inputs.ths_override_active;
  ElacComputer_B.yellow_low_pressure = ElacComputer_P.out_Y0.data.discrete_inputs.yellow_low_pressure;
  ElacComputer_B.capt_priority_takeover_pressed =
    ElacComputer_P.out_Y0.data.discrete_inputs.capt_priority_takeover_pressed;
  ElacComputer_B.fo_priority_takeover_pressed = ElacComputer_P.out_Y0.data.discrete_inputs.fo_priority_takeover_pressed;
  ElacComputer_B.blue_low_pressure = ElacComputer_P.out_Y0.data.discrete_inputs.blue_low_pressure;
  ElacComputer_B.green_low_pressure = ElacComputer_P.out_Y0.data.discrete_inputs.green_low_pressure;
  ElacComputer_B.elac_engaged_from_switch = ElacComputer_P.out_Y0.data.discrete_inputs.elac_engaged_from_switch;
  ElacComputer_B.normal_powersupply_lost = ElacComputer_P.out_Y0.data.discrete_inputs.normal_powersupply_lost;
  ElacComputer_B.capt_pitch_stick_pos = ElacComputer_P.out_Y0.data.analog_inputs.capt_pitch_stick_pos;
  ElacComputer_B.fo_pitch_stick_pos = ElacComputer_P.out_Y0.data.analog_inputs.fo_pitch_stick_pos;
  ElacComputer_B.capt_roll_stick_pos = ElacComputer_P.out_Y0.data.analog_inputs.capt_roll_stick_pos;
  ElacComputer_B.fo_roll_stick_pos = ElacComputer_P.out_Y0.data.analog_inputs.fo_roll_stick_pos;
  ElacComputer_B.left_elevator_pos_deg = ElacComputer_P.out_Y0.data.analog_inputs.left_elevator_pos_deg;
  ElacComputer_B.right_elevator_pos_deg = ElacComputer_P.out_Y0.data.analog_inputs.right_elevator_pos_deg;
  ElacComputer_B.ths_pos_deg = ElacComputer_P.out_Y0.data.analog_inputs.ths_pos_deg;
  ElacComputer_B.left_aileron_pos_deg = ElacComputer_P.out_Y0.data.analog_inputs.left_aileron_pos_deg;
  ElacComputer_B.right_aileron_pos_deg = ElacComputer_P.out_Y0.data.analog_inputs.right_aileron_pos_deg;
  ElacComputer_B.rudder_pedal_pos = ElacComputer_P.out_Y0.data.analog_inputs.rudder_pedal_pos;
  ElacComputer_B.load_factor_acc_1_g = ElacComputer_P.out_Y0.data.analog_inputs.load_factor_acc_1_g;
  ElacComputer_B.load_factor_acc_2_g = ElacComputer_P.out_Y0.data.analog_inputs.load_factor_acc_2_g;
  ElacComputer_B.blue_hyd_pressure_psi = ElacComputer_P.out_Y0.data.analog_inputs.blue_hyd_pressure_psi;
  ElacComputer_B.green_hyd_pressure_psi = ElacComputer_P.out_Y0.data.analog_inputs.green_hyd_pressure_psi;
  ElacComputer_B.yellow_hyd_pressure_psi = ElacComputer_P.out_Y0.data.analog_inputs.yellow_hyd_pressure_psi;
  ElacComputer_B.SSM_nl = ElacComputer_P.out_Y0.data.bus_inputs.adr_1_bus.altitude_standard_ft.SSM;
  ElacComputer_B.Data_ln = ElacComputer_P.out_Y0.data.bus_inputs.adr_1_bus.altitude_standard_ft.Data;
  ElacComputer_B.SSM_n3 = ElacComputer_P.out_Y0.data.bus_inputs.adr_1_bus.altitude_corrected_ft.SSM;
  ElacComputer_B.Data_fki = ElacComputer_P.out_Y0.data.bus_inputs.adr_1_bus.altitude_corrected_ft.Data;
  ElacComputer_B.SSM_a1 = ElacComputer_P.out_Y0.data.bus_inputs.adr_1_bus.mach.SSM;
  ElacComputer_B.Data_bv = ElacComputer_P.out_Y0.data.bus_inputs.adr_1_bus.mach.Data;
  ElacComputer_B.SSM_p1 = ElacComputer_P.out_Y0.data.bus_inputs.adr_1_bus.airspeed_computed_kn.SSM;
  ElacComputer_B.Data_m21 = ElacComputer_P.out_Y0.data.bus_inputs.adr_1_bus.airspeed_computed_kn.Data;
  ElacComputer_B.SSM_cn2 = ElacComputer_P.out_Y0.data.bus_inputs.adr_1_bus.airspeed_true_kn.SSM;
  ElacComputer_B.Data_nbg = ElacComputer_P.out_Y0.data.bus_inputs.adr_1_bus.airspeed_true_kn.Data;
  ElacComputer_B.SSM_an3 = ElacComputer_P.out_Y0.data.bus_inputs.adr_1_bus.vertical_speed_ft_min.SSM;
  ElacComputer_B.Data_l25 = ElacComputer_P.out_Y0.data.bus_inputs.adr_1_bus.vertical_speed_ft_min.Data;
  ElacComputer_B.SSM_c3 = ElacComputer_P.out_Y0.data.bus_inputs.adr_1_bus.aoa_corrected_deg.SSM;
  ElacComputer_B.Data_ki = ElacComputer_P.out_Y0.data.bus_inputs.adr_1_bus.aoa_corrected_deg.Data;
  ElacComputer_B.SSM_dp = ElacComputer_P.out_Y0.data.bus_inputs.adr_2_bus.altitude_standard_ft.SSM;
  ElacComputer_B.Data_p5p = ElacComputer_P.out_Y0.data.bus_inputs.adr_2_bus.altitude_standard_ft.Data;
  ElacComputer_B.SSM_boy = ElacComputer_P.out_Y0.data.bus_inputs.adr_2_bus.altitude_corrected_ft.SSM;
  ElacComputer_B.Data_nry = ElacComputer_P.out_Y0.data.bus_inputs.adr_2_bus.altitude_corrected_ft.Data;
  ElacComputer_B.SSM_lg = ElacComputer_P.out_Y0.data.bus_inputs.adr_2_bus.mach.SSM;
  ElacComputer_B.Data_mh = ElacComputer_P.out_Y0.data.bus_inputs.adr_2_bus.mach.Data;
  ElacComputer_B.SSM_cm = ElacComputer_P.out_Y0.data.bus_inputs.adr_2_bus.airspeed_computed_kn.SSM;
  ElacComputer_B.Data_ll = ElacComputer_P.out_Y0.data.bus_inputs.adr_2_bus.airspeed_computed_kn.Data;
  ElacComputer_B.SSM_hl = ElacComputer_P.out_Y0.data.bus_inputs.adr_2_bus.airspeed_true_kn.SSM;
  ElacComputer_B.Data_hy = ElacComputer_P.out_Y0.data.bus_inputs.adr_2_bus.airspeed_true_kn.Data;
  ElacComputer_B.SSM_irh = ElacComputer_P.out_Y0.data.bus_inputs.adr_2_bus.vertical_speed_ft_min.SSM;
  ElacComputer_B.Data_j04 = ElacComputer_P.out_Y0.data.bus_inputs.adr_2_bus.vertical_speed_ft_min.Data;
  ElacComputer_B.SSM_b42 = ElacComputer_P.out_Y0.data.bus_inputs.adr_2_bus.aoa_corrected_deg.SSM;
  ElacComputer_B.Data_pf = ElacComputer_P.out_Y0.data.bus_inputs.adr_2_bus.aoa_corrected_deg.Data;
  ElacComputer_B.SSM_anz = ElacComputer_P.out_Y0.data.bus_inputs.adr_3_bus.altitude_standard_ft.SSM;
  ElacComputer_B.Data_pl = ElacComputer_P.out_Y0.data.bus_inputs.adr_3_bus.altitude_standard_ft.Data;
  ElacComputer_B.SSM_d2 = ElacComputer_P.out_Y0.data.bus_inputs.adr_3_bus.altitude_corrected_ft.SSM;
  ElacComputer_B.Data_gb = ElacComputer_P.out_Y0.data.bus_inputs.adr_3_bus.altitude_corrected_ft.Data;
  ElacComputer_B.SSM_gov = ElacComputer_P.out_Y0.data.bus_inputs.adr_3_bus.mach.SSM;
  ElacComputer_B.Data_hq = ElacComputer_P.out_Y0.data.bus_inputs.adr_3_bus.mach.Data;
  ElacComputer_B.SSM_nb = ElacComputer_P.out_Y0.data.bus_inputs.adr_3_bus.airspeed_computed_kn.SSM;
  ElacComputer_B.Data_ai = ElacComputer_P.out_Y0.data.bus_inputs.adr_3_bus.airspeed_computed_kn.Data;
  ElacComputer_B.SSM_pe3 = ElacComputer_P.out_Y0.data.bus_inputs.adr_3_bus.airspeed_true_kn.SSM;
  ElacComputer_B.Data_gfr = ElacComputer_P.out_Y0.data.bus_inputs.adr_3_bus.airspeed_true_kn.Data;
  ElacComputer_B.SSM_jj = ElacComputer_P.out_Y0.data.bus_inputs.adr_3_bus.vertical_speed_ft_min.SSM;
  ElacComputer_B.Data_czp = ElacComputer_P.out_Y0.data.bus_inputs.adr_3_bus.vertical_speed_ft_min.Data;
  ElacComputer_B.SSM_jx = ElacComputer_P.out_Y0.data.bus_inputs.adr_3_bus.aoa_corrected_deg.SSM;
  ElacComputer_B.Data_fm = ElacComputer_P.out_Y0.data.bus_inputs.adr_3_bus.aoa_corrected_deg.Data;
  ElacComputer_B.SSM_npl = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.discrete_word_1.SSM;
  ElacComputer_B.Data_jsg = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.discrete_word_1.Data;
  ElacComputer_B.SSM = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.latitude_deg.SSM;
  ElacComputer_B.Data = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.latitude_deg.Data;
  ElacComputer_B.SSM_k = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.longitude_deg.SSM;
  ElacComputer_B.Data_f = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.longitude_deg.Data;
  ElacComputer_B.SSM_kx = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.ground_speed_kn.SSM;
  ElacComputer_B.Data_fw = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.ground_speed_kn.Data;
  ElacComputer_B.SSM_kxx = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.track_angle_true_deg.SSM;
  ElacComputer_B.Data_fwx = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.track_angle_true_deg.Data;
  ElacComputer_B.SSM_kxxt = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.heading_true_deg.SSM;
  ElacComputer_B.Data_fwxk = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.heading_true_deg.Data;
  ElacComputer_B.SSM_kxxta = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.wind_speed_kn.SSM;
  ElacComputer_B.Data_fwxkf = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.wind_speed_kn.Data;
  ElacComputer_B.SSM_kxxtac = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.wind_direction_true_deg.SSM;
  ElacComputer_B.Data_fwxkft = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.wind_direction_true_deg.Data;
  ElacComputer_B.SSM_kxxtac0 = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.track_angle_magnetic_deg.SSM;
  ElacComputer_B.Data_fwxkftc = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.track_angle_magnetic_deg.Data;
  ElacComputer_B.SSM_kxxtac0z = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.heading_magnetic_deg.SSM;
  ElacComputer_B.Data_fwxkftc3 = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.heading_magnetic_deg.Data;
  ElacComputer_B.SSM_kxxtac0zt = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.drift_angle_deg.SSM;
  ElacComputer_B.Data_fwxkftc3e = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.drift_angle_deg.Data;
  ElacComputer_B.SSM_kxxtac0ztg = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.flight_path_angle_deg.SSM;
  ElacComputer_B.Data_fwxkftc3ep = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.flight_path_angle_deg.Data;
  ElacComputer_B.SSM_kxxtac0ztgf = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.flight_path_accel_g.SSM;
  ElacComputer_B.Data_fwxkftc3epg = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.flight_path_accel_g.Data;
  ElacComputer_B.SSM_kxxtac0ztgf2 = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.pitch_angle_deg.SSM;
  ElacComputer_B.Data_fwxkftc3epgt = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.pitch_angle_deg.Data;
  ElacComputer_B.SSM_kxxtac0ztgf2u = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.roll_angle_deg.SSM;
  ElacComputer_B.Data_fwxkftc3epgtd = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.roll_angle_deg.Data;
  ElacComputer_B.SSM_kxxtac0ztgf2ux = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.body_pitch_rate_deg_s.SSM;
  ElacComputer_B.Data_fwxkftc3epgtdx = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.body_pitch_rate_deg_s.Data;
  ElacComputer_B.SSM_kxxtac0ztgf2uxn = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.body_roll_rate_deg_s.SSM;
  ElacComputer_B.Data_fwxkftc3epgtdxc = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.body_roll_rate_deg_s.Data;
  ElacComputer_B.SSM_ky = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.body_yaw_rate_deg_s.SSM;
  ElacComputer_B.Data_h = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.body_yaw_rate_deg_s.Data;
  ElacComputer_B.SSM_d = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.body_long_accel_g.SSM;
  ElacComputer_B.Data_e = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.body_long_accel_g.Data;
  ElacComputer_B.SSM_h = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.body_lat_accel_g.SSM;
  ElacComputer_B.Data_j = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.body_lat_accel_g.Data;
  ElacComputer_B.SSM_kb = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.body_normal_accel_g.SSM;
  ElacComputer_B.Data_d = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.body_normal_accel_g.Data;
  ElacComputer_B.SSM_p = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.track_angle_rate_deg_s.SSM;
  ElacComputer_B.Data_p = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.track_angle_rate_deg_s.Data;
  ElacComputer_B.SSM_di = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.pitch_att_rate_deg_s.SSM;
  ElacComputer_B.Data_i = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.pitch_att_rate_deg_s.Data;
  ElacComputer_B.SSM_j = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.roll_att_rate_deg_s.SSM;
  ElacComputer_B.Data_g = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.roll_att_rate_deg_s.Data;
  ElacComputer_B.SSM_i = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.inertial_alt_ft.SSM;
  ElacComputer_B.Data_a = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.inertial_alt_ft.Data;
  ElacComputer_B.SSM_g = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.along_track_horiz_acc_g.SSM;
  ElacComputer_B.Data_eb = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.along_track_horiz_acc_g.Data;
  ElacComputer_B.SSM_db = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.cross_track_horiz_acc_g.SSM;
  ElacComputer_B.Data_jo = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.cross_track_horiz_acc_g.Data;
  ElacComputer_B.SSM_n = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.vertical_accel_g.SSM;
  ElacComputer_B.Data_ex = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.vertical_accel_g.Data;
  ElacComputer_B.SSM_a = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.inertial_vertical_speed_ft_s.SSM;
  ElacComputer_B.Data_fd = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.inertial_vertical_speed_ft_s.Data;
  ElacComputer_B.SSM_ir = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.north_south_velocity_kn.SSM;
  ElacComputer_B.Data_ja = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.north_south_velocity_kn.Data;
  ElacComputer_B.SSM_hu = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.east_west_velocity_kn.SSM;
  ElacComputer_B.Data_k = ElacComputer_P.out_Y0.data.bus_inputs.ir_1_bus.east_west_velocity_kn.Data;
  ElacComputer_B.SSM_e = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.discrete_word_1.SSM;
  ElacComputer_B.Data_joy = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.discrete_word_1.Data;
  ElacComputer_B.SSM_gr = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.latitude_deg.SSM;
  ElacComputer_B.Data_h3 = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.latitude_deg.Data;
  ElacComputer_B.SSM_ev = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.longitude_deg.SSM;
  ElacComputer_B.Data_a0 = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.longitude_deg.Data;
  ElacComputer_B.SSM_l = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.ground_speed_kn.SSM;
  ElacComputer_B.Data_b = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.ground_speed_kn.Data;
  ElacComputer_B.SSM_ei = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.track_angle_true_deg.SSM;
  ElacComputer_B.Data_eq = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.track_angle_true_deg.Data;
  ElacComputer_B.SSM_an = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.heading_true_deg.SSM;
  ElacComputer_B.Data_iz = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.heading_true_deg.Data;
  ElacComputer_B.SSM_c = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.wind_speed_kn.SSM;
  ElacComputer_B.Data_j2 = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.wind_speed_kn.Data;
  ElacComputer_B.SSM_cb = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.wind_direction_true_deg.SSM;
  ElacComputer_B.Data_o = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.wind_direction_true_deg.Data;
  ElacComputer_B.SSM_lb = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.track_angle_magnetic_deg.SSM;
  ElacComputer_B.Data_m = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.track_angle_magnetic_deg.Data;
  ElacComputer_B.SSM_ia = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.heading_magnetic_deg.SSM;
  ElacComputer_B.Data_oq = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.heading_magnetic_deg.Data;
  ElacComputer_B.SSM_kyz = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.drift_angle_deg.SSM;
  ElacComputer_B.Data_fo = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.drift_angle_deg.Data;
  ElacComputer_B.SSM_as = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.flight_path_angle_deg.SSM;
  ElacComputer_B.Data_p1 = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.flight_path_angle_deg.Data;
  ElacComputer_B.SSM_is = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.flight_path_accel_g.SSM;
  ElacComputer_B.Data_p1y = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.flight_path_accel_g.Data;
  ElacComputer_B.SSM_ca = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.pitch_angle_deg.SSM;
  ElacComputer_B.Data_l = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.pitch_angle_deg.Data;
  ElacComputer_B.SSM_o = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.roll_angle_deg.SSM;
  ElacComputer_B.Data_kp = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.roll_angle_deg.Data;
  ElacComputer_B.SSM_ak = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.body_pitch_rate_deg_s.SSM;
  ElacComputer_B.Data_k0 = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.body_pitch_rate_deg_s.Data;
  ElacComputer_B.SSM_cbj = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.body_roll_rate_deg_s.SSM;
  ElacComputer_B.Data_pi = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.body_roll_rate_deg_s.Data;
  ElacComputer_B.SSM_cu = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.body_yaw_rate_deg_s.SSM;
  ElacComputer_B.Data_dm = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.body_yaw_rate_deg_s.Data;
  ElacComputer_B.SSM_nn = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.body_long_accel_g.SSM;
  ElacComputer_B.Data_f5 = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.body_long_accel_g.Data;
  ElacComputer_B.SSM_b = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.body_lat_accel_g.SSM;
  ElacComputer_B.Data_js = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.body_lat_accel_g.Data;
  ElacComputer_B.SSM_m = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.body_normal_accel_g.SSM;
  ElacComputer_B.Data_ee = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.body_normal_accel_g.Data;
  ElacComputer_B.SSM_f = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.track_angle_rate_deg_s.SSM;
  ElacComputer_B.Data_ig = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.track_angle_rate_deg_s.Data;
  ElacComputer_B.SSM_bp = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.pitch_att_rate_deg_s.SSM;
  ElacComputer_B.Data_mk = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.pitch_att_rate_deg_s.Data;
  ElacComputer_B.SSM_hb = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.roll_att_rate_deg_s.SSM;
  ElacComputer_B.Data_pu = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.roll_att_rate_deg_s.Data;
  ElacComputer_B.SSM_gz = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.inertial_alt_ft.SSM;
  ElacComputer_B.Data_ly = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.inertial_alt_ft.Data;
  ElacComputer_B.SSM_pv = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.along_track_horiz_acc_g.SSM;
  ElacComputer_B.Data_jq = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.along_track_horiz_acc_g.Data;
  ElacComputer_B.SSM_mf = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.cross_track_horiz_acc_g.SSM;
  ElacComputer_B.Data_o5 = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.cross_track_horiz_acc_g.Data;
  ElacComputer_B.SSM_m0 = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.vertical_accel_g.SSM;
  ElacComputer_B.Data_lyw = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.vertical_accel_g.Data;
  ElacComputer_B.SSM_kd = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.inertial_vertical_speed_ft_s.SSM;
  ElacComputer_B.Data_gq = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.inertial_vertical_speed_ft_s.Data;
  ElacComputer_B.SSM_pu = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.north_south_velocity_kn.SSM;
  ElacComputer_B.Data_n = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.north_south_velocity_kn.Data;
  ElacComputer_B.SSM_nv = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.east_west_velocity_kn.SSM;
  ElacComputer_B.Data_bq = ElacComputer_P.out_Y0.data.bus_inputs.ir_2_bus.east_west_velocity_kn.Data;
  ElacComputer_B.SSM_d5 = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.discrete_word_1.SSM;
  ElacComputer_B.Data_dmn = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.discrete_word_1.Data;
  ElacComputer_B.SSM_eo = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.latitude_deg.SSM;
  ElacComputer_B.Data_jn = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.latitude_deg.Data;
  ElacComputer_B.SSM_nd = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.longitude_deg.SSM;
  ElacComputer_B.Data_c = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.longitude_deg.Data;
  ElacComputer_B.SSM_bq = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.ground_speed_kn.SSM;
  ElacComputer_B.Data_lx = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.ground_speed_kn.Data;
  ElacComputer_B.SSM_hi = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.track_angle_true_deg.SSM;
  ElacComputer_B.Data_jb = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.track_angle_true_deg.Data;
  ElacComputer_B.SSM_mm = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.heading_true_deg.SSM;
  ElacComputer_B.Data_fn = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.heading_true_deg.Data;
  ElacComputer_B.SSM_kz = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.wind_speed_kn.SSM;
  ElacComputer_B.Data_od = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.wind_speed_kn.Data;
  ElacComputer_B.SSM_il = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.wind_direction_true_deg.SSM;
  ElacComputer_B.Data_ez = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.wind_direction_true_deg.Data;
  ElacComputer_B.SSM_i2 = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.track_angle_magnetic_deg.SSM;
  ElacComputer_B.Data_pw = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.track_angle_magnetic_deg.Data;
  ElacComputer_B.SSM_ah = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.heading_magnetic_deg.SSM;
  ElacComputer_B.Data_m2 = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.heading_magnetic_deg.Data;
  ElacComputer_B.SSM_en = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.drift_angle_deg.SSM;
  ElacComputer_B.Data_ek = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.drift_angle_deg.Data;
  ElacComputer_B.SSM_dq = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.flight_path_angle_deg.SSM;
  ElacComputer_B.Data_iy = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.flight_path_angle_deg.Data;
  ElacComputer_B.SSM_px = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.flight_path_accel_g.SSM;
  ElacComputer_B.Data_lk = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.flight_path_accel_g.Data;
  ElacComputer_B.SSM_lbo = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.pitch_angle_deg.SSM;
  ElacComputer_B.Data_ca = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.pitch_angle_deg.Data;
  ElacComputer_B.SSM_p5 = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.roll_angle_deg.SSM;
  ElacComputer_B.Data_pix = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.roll_angle_deg.Data;
  ElacComputer_B.SSM_mk = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.body_pitch_rate_deg_s.SSM;
  ElacComputer_B.Data_di = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.body_pitch_rate_deg_s.Data;
  ElacComputer_B.SSM_mu = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.body_roll_rate_deg_s.SSM;
  ElacComputer_B.Data_lz = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.body_roll_rate_deg_s.Data;
  ElacComputer_B.SSM_cbl = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.body_yaw_rate_deg_s.SSM;
  ElacComputer_B.Data_lu = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.body_yaw_rate_deg_s.Data;
  ElacComputer_B.SSM_gzd = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.body_long_accel_g.SSM;
  ElacComputer_B.Data_dc = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.body_long_accel_g.Data;
  ElacComputer_B.SSM_mo = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.body_lat_accel_g.SSM;
  ElacComputer_B.Data_gc = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.body_lat_accel_g.Data;
  ElacComputer_B.SSM_me = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.body_normal_accel_g.SSM;
  ElacComputer_B.Data_am = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.body_normal_accel_g.Data;
  ElacComputer_B.SSM_mj = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.track_angle_rate_deg_s.SSM;
  ElacComputer_B.Data_mo = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.track_angle_rate_deg_s.Data;
  ElacComputer_B.SSM_a5 = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.pitch_att_rate_deg_s.SSM;
  ElacComputer_B.Data_dg = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.pitch_att_rate_deg_s.Data;
  ElacComputer_B.SSM_bt = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.roll_att_rate_deg_s.SSM;
  ElacComputer_B.Data_e1 = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.roll_att_rate_deg_s.Data;
  ElacComputer_B.SSM_om = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.inertial_alt_ft.SSM;
  ElacComputer_B.Data_fp = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.inertial_alt_ft.Data;
  ElacComputer_B.SSM_ar = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.along_track_horiz_acc_g.SSM;
  ElacComputer_B.Data_ns = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.along_track_horiz_acc_g.Data;
  ElacComputer_B.SSM_ce = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.cross_track_horiz_acc_g.SSM;
  ElacComputer_B.Data_m3 = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.cross_track_horiz_acc_g.Data;
  ElacComputer_B.SSM_ed = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.vertical_accel_g.SSM;
  ElacComputer_B.Data_oj = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.vertical_accel_g.Data;
  ElacComputer_B.SSM_jh = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.inertial_vertical_speed_ft_s.SSM;
  ElacComputer_B.Data_jy = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.inertial_vertical_speed_ft_s.Data;
  ElacComputer_B.SSM_je = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.north_south_velocity_kn.SSM;
  ElacComputer_B.Data_j1 = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.north_south_velocity_kn.Data;
  ElacComputer_B.SSM_jt = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.east_west_velocity_kn.SSM;
  ElacComputer_B.Data_fc = ElacComputer_P.out_Y0.data.bus_inputs.ir_3_bus.east_west_velocity_kn.Data;
  ElacComputer_B.SSM_cui = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.fac_weight_lbs.SSM;
  ElacComputer_B.Data_of = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.fac_weight_lbs.Data;
  ElacComputer_B.SSM_mq = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.fm_weight_lbs.SSM;
  ElacComputer_B.Data_lg = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.fm_weight_lbs.Data;
  ElacComputer_B.SSM_ni = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.fac_cg_percent.SSM;
  ElacComputer_B.Data_n4 = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.fac_cg_percent.Data;
  ElacComputer_B.SSM_df = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.fm_cg_percent.SSM;
  ElacComputer_B.Data_ot = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.fm_cg_percent.Data;
  ElacComputer_B.SSM_oe = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.fg_radio_height_ft.SSM;
  ElacComputer_B.Data_gv = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.fg_radio_height_ft.Data;
  ElacComputer_B.SSM_ha = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.discrete_word_4.SSM;
  ElacComputer_B.Data_ou = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.discrete_word_4.Data;
  ElacComputer_B.SSM_op = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.ats_discrete_word.SSM;
  ElacComputer_B.Data_dh = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.ats_discrete_word.Data;
  ElacComputer_B.SSM_a50 = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.discrete_word_3.SSM;
  ElacComputer_B.Data_ph = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.discrete_word_3.Data;
  ElacComputer_B.SSM_og = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.discrete_word_1.SSM;
  ElacComputer_B.Data_gs = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.discrete_word_1.Data;
  ElacComputer_B.SSM_a4 = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.discrete_word_2.SSM;
  ElacComputer_B.Data_fd4 = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.discrete_word_2.Data;
  ElacComputer_B.SSM_bv = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.approach_spd_target_kn.SSM;
  ElacComputer_B.Data_hm = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.approach_spd_target_kn.Data;
  ElacComputer_B.SSM_bo = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.delta_p_ail_cmd_deg.SSM;
  ElacComputer_B.Data_i2 = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.delta_p_ail_cmd_deg.Data;
  ElacComputer_B.SSM_d1 = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.delta_p_splr_cmd_deg.SSM;
  ElacComputer_B.Data_og = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.delta_p_splr_cmd_deg.Data;
  ElacComputer_B.SSM_hy = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.delta_r_cmd_deg.SSM;
  ElacComputer_B.Data_fv = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.delta_r_cmd_deg.Data;
  ElacComputer_B.SSM_gi = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.delta_nose_wheel_cmd_deg.SSM;
  ElacComputer_B.Data_oc = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.delta_nose_wheel_cmd_deg.Data;
  ElacComputer_B.SSM_pp = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.delta_q_cmd_deg.SSM;
  ElacComputer_B.Data_kq = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.delta_q_cmd_deg.Data;
  ElacComputer_B.SSM_iab = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.n1_left_percent.SSM;
  ElacComputer_B.Data_ne = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.n1_left_percent.Data;
  ElacComputer_B.SSM_jtv = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.n1_right_percent.SSM;
  ElacComputer_B.Data_it = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_1_bus.n1_right_percent.Data;
  ElacComputer_B.SSM_fy = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.fac_weight_lbs.SSM;
  ElacComputer_B.Data_ch = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.fac_weight_lbs.Data;
  ElacComputer_B.SSM_d4 = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.fm_weight_lbs.SSM;
  ElacComputer_B.Data_bb = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.fm_weight_lbs.Data;
  ElacComputer_B.SSM_ars = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.fac_cg_percent.SSM;
  ElacComputer_B.Data_ol = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.fac_cg_percent.Data;
  ElacComputer_B.SSM_din = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.fm_cg_percent.SSM;
  ElacComputer_B.Data_hw = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.fm_cg_percent.Data;
  ElacComputer_B.SSM_m3 = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.fg_radio_height_ft.SSM;
  ElacComputer_B.Data_hs = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.fg_radio_height_ft.Data;
  ElacComputer_B.SSM_np = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.discrete_word_4.SSM;
  ElacComputer_B.Data_fj = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.discrete_word_4.Data;
  ElacComputer_B.SSM_ax = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.ats_discrete_word.SSM;
  ElacComputer_B.Data_ky = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.ats_discrete_word.Data;
  ElacComputer_B.SSM_cl = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.discrete_word_3.SSM;
  ElacComputer_B.Data_h5 = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.discrete_word_3.Data;
  ElacComputer_B.SSM_es = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.discrete_word_1.SSM;
  ElacComputer_B.Data_ku = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.discrete_word_1.Data;
  ElacComputer_B.SSM_gi1 = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.discrete_word_2.SSM;
  ElacComputer_B.Data_jp = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.discrete_word_2.Data;
  ElacComputer_B.SSM_jz = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.approach_spd_target_kn.SSM;
  ElacComputer_B.Data_nu = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.approach_spd_target_kn.Data;
  ElacComputer_B.SSM_kt = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.delta_p_ail_cmd_deg.SSM;
  ElacComputer_B.Data_br = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.delta_p_ail_cmd_deg.Data;
  ElacComputer_B.SSM_ds = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.delta_p_splr_cmd_deg.SSM;
  ElacComputer_B.Data_ae = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.delta_p_splr_cmd_deg.Data;
  ElacComputer_B.SSM_eg = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.delta_r_cmd_deg.SSM;
  ElacComputer_B.Data_pe = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.delta_r_cmd_deg.Data;
  ElacComputer_B.SSM_a0 = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.delta_nose_wheel_cmd_deg.SSM;
  ElacComputer_B.Data_fy = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.delta_nose_wheel_cmd_deg.Data;
  ElacComputer_B.SSM_cv = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.delta_q_cmd_deg.SSM;
  ElacComputer_B.Data_na = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.delta_q_cmd_deg.Data;
  ElacComputer_B.SSM_ea = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.n1_left_percent.SSM;
  ElacComputer_B.Data_my = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.n1_left_percent.Data;
  ElacComputer_B.SSM_p4 = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.n1_right_percent.SSM;
  ElacComputer_B.Data_i4 = ElacComputer_P.out_Y0.data.bus_inputs.fmgc_2_bus.n1_right_percent.Data;
  ElacComputer_B.SSM_m2 = ElacComputer_P.out_Y0.data.bus_inputs.ra_1_bus.radio_height_ft.SSM;
  ElacComputer_B.Data_cx = ElacComputer_P.out_Y0.data.bus_inputs.ra_1_bus.radio_height_ft.Data;
  ElacComputer_B.SSM_bt0 = ElacComputer_P.out_Y0.data.bus_inputs.ra_2_bus.radio_height_ft.SSM;
  ElacComputer_B.Data_nz = ElacComputer_P.out_Y0.data.bus_inputs.ra_2_bus.radio_height_ft.Data;
  ElacComputer_B.SSM_nr = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_1_bus.slat_flap_component_status_word.SSM;
  ElacComputer_B.Data_id = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_1_bus.slat_flap_component_status_word.Data;
  ElacComputer_B.SSM_fr = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_1_bus.slat_flap_system_status_word.SSM;
  ElacComputer_B.Data_o2 = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_1_bus.slat_flap_system_status_word.Data;
  ElacComputer_B.SSM_cc = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_1_bus.slat_flap_actual_position_word.SSM;
  ElacComputer_B.Data_gqq = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_1_bus.slat_flap_actual_position_word.Data;
  ElacComputer_B.SSM_lm = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_1_bus.slat_actual_position_deg.SSM;
  ElacComputer_B.Data_md = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_1_bus.slat_actual_position_deg.Data;
  ElacComputer_B.SSM_mkm = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_1_bus.flap_actual_position_deg.SSM;
  ElacComputer_B.Data_cz = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_1_bus.flap_actual_position_deg.Data;
  ElacComputer_B.SSM_jhd = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_2_bus.slat_flap_component_status_word.SSM;
  ElacComputer_B.Data_pm = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_2_bus.slat_flap_component_status_word.Data;
  ElacComputer_B.SSM_av = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_2_bus.slat_flap_system_status_word.SSM;
  ElacComputer_B.Data_bj = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_2_bus.slat_flap_system_status_word.Data;
  ElacComputer_B.SSM_ira = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_2_bus.slat_flap_actual_position_word.SSM;
  ElacComputer_B.Data_ox = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_2_bus.slat_flap_actual_position_word.Data;
  ElacComputer_B.SSM_ge = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_2_bus.slat_actual_position_deg.SSM;
  ElacComputer_B.Data_pe5 = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_2_bus.slat_actual_position_deg.Data;
  ElacComputer_B.SSM_lv = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_2_bus.flap_actual_position_deg.SSM;
  ElacComputer_B.Data_jj = ElacComputer_P.out_Y0.data.bus_inputs.sfcc_2_bus.flap_actual_position_deg.Data;
  ElacComputer_B.SSM_cg = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.efcs_status_word_1.SSM;
  ElacComputer_B.Data_p5 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.efcs_status_word_1.Data;
  ElacComputer_B.SSM_be = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.efcs_status_word_2.SSM;
  ElacComputer_B.Data_ekl = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.efcs_status_word_2.Data;
  ElacComputer_B.SSM_axb = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.efcs_status_word_3.SSM;
  ElacComputer_B.Data_nd = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.efcs_status_word_3.Data;
  ElacComputer_B.SSM_nz = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.efcs_status_word_4.SSM;
  ElacComputer_B.Data_n2 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.efcs_status_word_4.Data;
  ElacComputer_B.SSM_cx = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.efcs_status_word_5.SSM;
  ElacComputer_B.Data_dl = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.efcs_status_word_5.Data;
  ElacComputer_B.SSM_gh = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.capt_roll_command_deg.SSM;
  ElacComputer_B.Data_gs2 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.capt_roll_command_deg.Data;
  ElacComputer_B.SSM_ks = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.fo_roll_command_deg.SSM;
  ElacComputer_B.Data_h4 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.fo_roll_command_deg.Data;
  ElacComputer_B.SSM_pw = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.rudder_pedal_position_deg.SSM;
  ElacComputer_B.Data_e3 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.rudder_pedal_position_deg.Data;
  ElacComputer_B.SSM_fh = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.capt_pitch_command_deg.SSM;
  ElacComputer_B.Data_f5h = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.capt_pitch_command_deg.Data;
  ElacComputer_B.SSM_gzn = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.fo_pitch_command_deg.SSM;
  ElacComputer_B.Data_an = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.fo_pitch_command_deg.Data;
  ElacComputer_B.SSM_oo = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.aileron_left_pos_deg.SSM;
  ElacComputer_B.Data_i4o = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.aileron_left_pos_deg.Data;
  ElacComputer_B.SSM_evh = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.elevator_left_pos_deg.SSM;
  ElacComputer_B.Data_af = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.elevator_left_pos_deg.Data;
  ElacComputer_B.SSM_cn = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.aileron_right_pos_deg.SSM;
  ElacComputer_B.Data_bm = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.aileron_right_pos_deg.Data;
  ElacComputer_B.SSM_co = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.elevator_right_pos_deg.SSM;
  ElacComputer_B.Data_dk = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.elevator_right_pos_deg.Data;
  ElacComputer_B.SSM_pe = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.horiz_stab_trim_pos_deg.SSM;
  ElacComputer_B.Data_nv = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.horiz_stab_trim_pos_deg.Data;
  ElacComputer_B.SSM_cgz = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_1_left_pos_deg.SSM;
  ElacComputer_B.Data_jpf = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_1_left_pos_deg.Data;
  ElacComputer_B.SSM_fw = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_2_left_pos_deg.SSM;
  ElacComputer_B.Data_i5 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_2_left_pos_deg.Data;
  ElacComputer_B.SSM_h4 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_3_left_pos_deg.SSM;
  ElacComputer_B.Data_k2 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_3_left_pos_deg.Data;
  ElacComputer_B.SSM_cb3 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_4_left_pos_deg.SSM;
  ElacComputer_B.Data_as = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_4_left_pos_deg.Data;
  ElacComputer_B.SSM_pj = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_5_left_pos_deg.SSM;
  ElacComputer_B.Data_gk = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_5_left_pos_deg.Data;
  ElacComputer_B.SSM_dv = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_1_right_pos_deg.SSM;
  ElacComputer_B.Data_jl = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_1_right_pos_deg.Data;
  ElacComputer_B.SSM_i4 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_2_right_pos_deg.SSM;
  ElacComputer_B.Data_e32 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_2_right_pos_deg.Data;
  ElacComputer_B.SSM_fm = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_3_right_pos_deg.SSM;
  ElacComputer_B.Data_ih = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_3_right_pos_deg.Data;
  ElacComputer_B.SSM_e5 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_4_right_pos_deg.SSM;
  ElacComputer_B.Data_du = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_4_right_pos_deg.Data;
  ElacComputer_B.SSM_bf = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_5_right_pos_deg.SSM;
  ElacComputer_B.Data_nx = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_1_bus.spoiler_5_right_pos_deg.Data;
  ElacComputer_B.SSM_fd = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.efcs_status_word_1.SSM;
  ElacComputer_B.Data_n0 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.efcs_status_word_1.Data;
  ElacComputer_B.SSM_fv = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.efcs_status_word_2.SSM;
  ElacComputer_B.Data_eqi = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.efcs_status_word_2.Data;
  ElacComputer_B.SSM_dt = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.efcs_status_word_3.SSM;
  ElacComputer_B.Data_om = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.efcs_status_word_3.Data;
  ElacComputer_B.SSM_j5 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.efcs_status_word_4.SSM;
  ElacComputer_B.Data_nr = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.efcs_status_word_4.Data;
  ElacComputer_B.SSM_ng = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.efcs_status_word_5.SSM;
  ElacComputer_B.Data_p3 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.efcs_status_word_5.Data;
  ElacComputer_B.SSM_cs = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.capt_roll_command_deg.SSM;
  ElacComputer_B.Data_nb = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.capt_roll_command_deg.Data;
  ElacComputer_B.SSM_ls = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.fo_roll_command_deg.SSM;
  ElacComputer_B.Data_hd = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.fo_roll_command_deg.Data;
  ElacComputer_B.SSM_dg = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.rudder_pedal_position_deg.SSM;
  ElacComputer_B.Data_al = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.rudder_pedal_position_deg.Data;
  ElacComputer_B.SSM_d3 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.capt_pitch_command_deg.SSM;
  ElacComputer_B.Data_gu = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.capt_pitch_command_deg.Data;
  ElacComputer_B.SSM_p2 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.fo_pitch_command_deg.SSM;
  ElacComputer_B.Data_ix = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.fo_pitch_command_deg.Data;
  ElacComputer_B.SSM_bo0 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.aileron_left_pos_deg.SSM;
  ElacComputer_B.Data_do = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.aileron_left_pos_deg.Data;
  ElacComputer_B.SSM_bc = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.elevator_left_pos_deg.SSM;
  ElacComputer_B.Data_hu = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.elevator_left_pos_deg.Data;
  ElacComputer_B.SSM_h0 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.aileron_right_pos_deg.SSM;
  ElacComputer_B.Data_pm1 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.aileron_right_pos_deg.Data;
  ElacComputer_B.SSM_giz = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.elevator_right_pos_deg.SSM;
  ElacComputer_B.Data_i2y = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.elevator_right_pos_deg.Data;
  ElacComputer_B.SSM_mqp = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.horiz_stab_trim_pos_deg.SSM;
  ElacComputer_B.Data_pg = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.horiz_stab_trim_pos_deg.Data;
  ElacComputer_B.SSM_ba = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_1_left_pos_deg.SSM;
  ElacComputer_B.Data_ni = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_1_left_pos_deg.Data;
  ElacComputer_B.SSM_in = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_2_left_pos_deg.SSM;
  ElacComputer_B.Data_fr = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_2_left_pos_deg.Data;
  ElacComputer_B.SSM_ff = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_3_left_pos_deg.SSM;
  ElacComputer_B.Data_cn = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_3_left_pos_deg.Data;
  ElacComputer_B.SSM_ic = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_4_left_pos_deg.SSM;
  ElacComputer_B.Data_nxl = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_4_left_pos_deg.Data;
  ElacComputer_B.SSM_fs = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_5_left_pos_deg.SSM;
  ElacComputer_B.Data_jh = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_5_left_pos_deg.Data;
  ElacComputer_B.SSM_ja = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_1_right_pos_deg.SSM;
  ElacComputer_B.Data_gl = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_1_right_pos_deg.Data;
  ElacComputer_B.SSM_js = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_2_right_pos_deg.SSM;
  ElacComputer_B.Data_gn = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_2_right_pos_deg.Data;
  ElacComputer_B.SSM_is3 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_3_right_pos_deg.SSM;
  ElacComputer_B.Data_myb = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_3_right_pos_deg.Data;
  ElacComputer_B.SSM_ag = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_4_right_pos_deg.SSM;
  ElacComputer_B.Data_l2 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_4_right_pos_deg.Data;
  ElacComputer_B.SSM_f5 = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_5_right_pos_deg.SSM;
  ElacComputer_B.Data_o5o = ElacComputer_P.out_Y0.data.bus_inputs.fcdc_2_bus.spoiler_5_right_pos_deg.Data;
  ElacComputer_B.SSM_ph = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.left_spoiler_1_position_deg.SSM;
  ElacComputer_B.Data_l5 = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.left_spoiler_1_position_deg.Data;
  ElacComputer_B.SSM_jw = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.right_spoiler_1_position_deg.SSM;
  ElacComputer_B.Data_dc2 = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.right_spoiler_1_position_deg.Data;
  ElacComputer_B.SSM_jy = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.left_spoiler_2_position_deg.SSM;
  ElacComputer_B.Data_gr = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.left_spoiler_2_position_deg.Data;
  ElacComputer_B.SSM_j1 = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.right_spoiler_2_position_deg.SSM;
  ElacComputer_B.Data_gp = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.right_spoiler_2_position_deg.Data;
  ElacComputer_B.SSM_ov = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.left_elevator_position_deg.SSM;
  ElacComputer_B.Data_i3 = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.left_elevator_position_deg.Data;
  ElacComputer_B.SSM_mx = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.right_elevator_position_deg.SSM;
  ElacComputer_B.Data_et = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.right_elevator_position_deg.Data;
  ElacComputer_B.SSM_b4 = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.ths_position_deg.SSM;
  ElacComputer_B.Data_mc = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.ths_position_deg.Data;
  ElacComputer_B.SSM_gb = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.left_sidestick_pitch_command_deg.SSM;
  ElacComputer_B.Data_k3 = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.left_sidestick_pitch_command_deg.Data;
  ElacComputer_B.SSM_oh = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.right_sidestick_pitch_command_deg.SSM;
  ElacComputer_B.Data_f2 = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.right_sidestick_pitch_command_deg.Data;
  ElacComputer_B.SSM_mm5 = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.left_sidestick_roll_command_deg.SSM;
  ElacComputer_B.Data_gh = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.left_sidestick_roll_command_deg.Data;
  ElacComputer_B.SSM_br = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.right_sidestick_roll_command_deg.SSM;
  ElacComputer_B.Data_ed = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.right_sidestick_roll_command_deg.Data;
  ElacComputer_B.SSM_c2 = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.speed_brake_lever_command_deg.SSM;
  ElacComputer_B.Data_o2j = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.speed_brake_lever_command_deg.Data;
  ElacComputer_B.SSM_hc = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.thrust_lever_angle_1_deg.SSM;
  ElacComputer_B.Data_i43 = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.thrust_lever_angle_1_deg.Data;
  ElacComputer_B.SSM_ktr = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.thrust_lever_angle_2_deg.SSM;
  ElacComputer_B.Data_ic = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.thrust_lever_angle_2_deg.Data;
  ElacComputer_B.SSM_gl = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.discrete_status_word_1.SSM;
  ElacComputer_B.Data_ak = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.discrete_status_word_1.Data;
  ElacComputer_B.SSM_my = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.discrete_status_word_2.SSM;
  ElacComputer_B.Data_jg = ElacComputer_P.out_Y0.data.bus_inputs.sec_1_bus.discrete_status_word_2.Data;
  ElacComputer_B.SSM_j3 = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.left_spoiler_1_position_deg.SSM;
  ElacComputer_B.Data_cu = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.left_spoiler_1_position_deg.Data;
  ElacComputer_B.SSM_go = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.right_spoiler_1_position_deg.SSM;
  ElacComputer_B.Data_ep = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.right_spoiler_1_position_deg.Data;
  ElacComputer_B.SSM_e5c = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.left_spoiler_2_position_deg.SSM;
  ElacComputer_B.Data_d3 = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.left_spoiler_2_position_deg.Data;
  ElacComputer_B.SSM_dk = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.right_spoiler_2_position_deg.SSM;
  ElacComputer_B.Data_bt = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.right_spoiler_2_position_deg.Data;
  ElacComputer_B.SSM_evc = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.left_elevator_position_deg.SSM;
  ElacComputer_B.Data_e0 = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.left_elevator_position_deg.Data;
  ElacComputer_B.SSM_kk = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.right_elevator_position_deg.SSM;
  ElacComputer_B.Data_jl3 = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.right_elevator_position_deg.Data;
  ElacComputer_B.SSM_af = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.ths_position_deg.SSM;
  ElacComputer_B.Data_nm = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.ths_position_deg.Data;
  ElacComputer_B.SSM_npr = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.left_sidestick_pitch_command_deg.SSM;
  ElacComputer_B.Data_ia = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.left_sidestick_pitch_command_deg.Data;
  ElacComputer_B.SSM_ew = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.right_sidestick_pitch_command_deg.SSM;
  ElacComputer_B.Data_j0 = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.right_sidestick_pitch_command_deg.Data;
  ElacComputer_B.SSM_lt = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.left_sidestick_roll_command_deg.SSM;
  ElacComputer_B.Data_bs = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.left_sidestick_roll_command_deg.Data;
  ElacComputer_B.SSM_ger = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.right_sidestick_roll_command_deg.SSM;
  ElacComputer_B.Data_hp = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.right_sidestick_roll_command_deg.Data;
  ElacComputer_B.SSM_pxo = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.speed_brake_lever_command_deg.SSM;
  ElacComputer_B.Data_ct = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.speed_brake_lever_command_deg.Data;
  ElacComputer_B.SSM_co2 = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.thrust_lever_angle_1_deg.SSM;
  ElacComputer_B.Data_pc = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.thrust_lever_angle_1_deg.Data;
  ElacComputer_B.SSM_ny = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.thrust_lever_angle_2_deg.SSM;
  ElacComputer_B.Data_nzt = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.thrust_lever_angle_2_deg.Data;
  ElacComputer_B.SSM_l4 = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.discrete_status_word_1.SSM;
  ElacComputer_B.Data_c0 = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.discrete_status_word_1.Data;
  ElacComputer_B.SSM_nm = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.discrete_status_word_2.SSM;
  ElacComputer_B.Data_ojg = ElacComputer_P.out_Y0.data.bus_inputs.sec_2_bus.discrete_status_word_2.Data;
  ElacComputer_B.SSM_nh = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.left_aileron_position_deg.SSM;
  ElacComputer_B.Data_lm = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.left_aileron_position_deg.Data;
  ElacComputer_B.SSM_dl = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.right_aileron_position_deg.SSM;
  ElacComputer_B.Data_fz = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.right_aileron_position_deg.Data;
  ElacComputer_B.SSM_dx = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.left_elevator_position_deg.SSM;
  ElacComputer_B.Data_oz = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.left_elevator_position_deg.Data;
  ElacComputer_B.SSM_a5h = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.right_elevator_position_deg.SSM;
  ElacComputer_B.Data_gf = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.right_elevator_position_deg.Data;
  ElacComputer_B.SSM_fl = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.ths_position_deg.SSM;
  ElacComputer_B.Data_nn = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.ths_position_deg.Data;
  ElacComputer_B.SSM_p3 = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.left_sidestick_pitch_command_deg.SSM;
  ElacComputer_B.Data_a0z = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.left_sidestick_pitch_command_deg.Data;
  ElacComputer_B.SSM_ns = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.right_sidestick_pitch_command_deg.SSM;
  ElacComputer_B.Data_fk = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.right_sidestick_pitch_command_deg.Data;
  ElacComputer_B.SSM_bm = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.left_sidestick_roll_command_deg.SSM;
  ElacComputer_B.Data_bu = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.left_sidestick_roll_command_deg.Data;
  ElacComputer_B.SSM_grm = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.right_sidestick_roll_command_deg.SSM;
  ElacComputer_B.Data_o23 = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.right_sidestick_roll_command_deg.Data;
  ElacComputer_B.SSM_gzm = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.rudder_pedal_position_deg.SSM;
  ElacComputer_B.Data_g3 = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.rudder_pedal_position_deg.Data;
  ElacComputer_B.SSM_oi = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.aileron_command_deg.SSM;
  ElacComputer_B.Data_icc = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.aileron_command_deg.Data;
  ElacComputer_B.SSM_aa = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.roll_spoiler_command_deg.SSM;
  ElacComputer_B.Data_pwf = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.roll_spoiler_command_deg.Data;
  ElacComputer_B.SSM_fvk = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.yaw_damper_command_deg.SSM;
  ElacComputer_B.Data_gvk = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.yaw_damper_command_deg.Data;
  ElacComputer_B.SSM_lw =
    ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.elevator_double_pressurization_command_deg.SSM;
  ElacComputer_B.Data_ka =
    ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.elevator_double_pressurization_command_deg.Data;
  ElacComputer_B.SSM_fa = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.discrete_status_word_1.SSM;
  ElacComputer_B.Data_mp = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.discrete_status_word_1.Data;
  ElacComputer_B.SSM_lbx = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.discrete_status_word_2.SSM;
  ElacComputer_B.Data_m4 = ElacComputer_P.out_Y0.data.bus_inputs.elac_opp_bus.discrete_status_word_2.Data;
  ElacComputer_B.laws = ElacComputer_P.out_Y0.laws;
  ElacComputer_B.logic = ElacComputer_P.out_Y0.logic;
  ElacComputer_Y.out.discrete_outputs = ElacComputer_P.out_Y0.discrete_outputs;
  ElacComputer_Y.out.analog_outputs = ElacComputer_P.out_Y0.analog_outputs;
  ElacComputer_Y.out.bus_outputs = ElacComputer_P.out_Y0.bus_outputs;
}

void ElacComputer::terminate()
{
}

ElacComputer::ElacComputer():
  ElacComputer_U(),
  ElacComputer_Y(),
  ElacComputer_B(),
  ElacComputer_DWork()
{
}

ElacComputer::~ElacComputer()
{
}
