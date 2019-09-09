///////////////////////////////////////////////////////////////////////////////
// BSD 3-Clause License
//
// Copyright (C) 2018-2019, LAAS-CNRS
// Copyright note valid unless otherwise stated in individual files.
// All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#include "crocoddyl/core/solvers/fddp.hpp"

namespace crocoddyl {

SolverFDDP::SolverFDDP(ShootingProblem& problem) : SolverDDP(problem), dg_(0), dq_(0), dv_(0), th_acceptNegStep_(2) {}

SolverFDDP::~SolverFDDP() {}

bool SolverFDDP::solve(const std::vector<Eigen::VectorXd>& init_xs, const std::vector<Eigen::VectorXd>& init_us,
                       const unsigned int& maxiter, const bool& is_feasible, const double& reginit) {
  setCandidate(init_xs, init_us, is_feasible);

  if (std::isnan(reginit)) {
    xreg_ = regmin_;
    ureg_ = regmin_;
  } else {
    xreg_ = reginit;
    ureg_ = reginit;
  }
  was_feasible_ = false;

  bool recalc = true;
  for (iter_ = 0; iter_ < maxiter; ++iter_) {
    while (true) {
      try {
        computeDirection(recalc);
      } catch (const char* msg) {
        recalc = false;
        increaseRegularization();
        if (xreg_ == regmax_) {
          return false;
        } else {
          continue;
        }
      }
      break;
    }
    updateExpectedImprovement();

    // We need to recalculate the derivatives when the step length passes
    recalc = false;
    for (std::vector<double>::const_iterator it = alphas_.begin(); it != alphas_.end(); ++it) {
      steplength_ = *it;

      try {
        dV_ = tryStep(steplength_);
      } catch (const char* msg) {
        continue;
      }
      expectedImprovement();
      dVexp_ = steplength_ * (d_[0] + 0.5 * steplength_ * d_[1]);

      if (dVexp_ > 0) {  // descend direction
        if (d_[0] < th_grad_ || dV_ > th_acceptstep_ * dVexp_) {
          // Accept step

          was_feasible_ = is_feasible_;
          setCandidate(xs_try_, us_try_, (was_feasible_) || (steplength_ == 1));
          cost_ = cost_try_;
          recalc = true;
          break;
        }
      } else {
        if (d_[0] < th_grad_ || dV_ < th_acceptNegStep_ * dVexp_) {
          // accept step
          was_feasible_ = is_feasible_;
          setCandidate(xs_try_, us_try_, (was_feasible_) || (steplength_ == 1));
          cost_ = cost_try_;
          recalc = true;
          break;
        }
      }
    }
    if (steplength_ > th_step_) {
      decreaseRegularization();
    }
    if (steplength_ == alphas_.back()) {
      increaseRegularization();
      if (xreg_ == regmax_) {
        return false;
      }
    }
    stoppingCriteria();

    unsigned int const& n_callbacks = static_cast<unsigned int>(callbacks_.size());
    for (unsigned int c = 0; c < n_callbacks; ++c) {
      CallbackAbstract& callback = *callbacks_[c];
      callback(*this);
    }

    if (was_feasible_ && stop_ < th_stop_) {
      return true;
    }
  }
  return false;
}

void SolverFDDP::updateExpectedImprovement() {
  dg_ = 0;
  dq_ = 0;
  unsigned int const& T = this->problem_.get_T();
  if (!is_feasible_) {
    dg_ -= Vx_.back().transpose() * gaps_.back();
    dq_ += gaps_.back().transpose() * Vxx_.back() * gaps_.back();
  }
  for (unsigned int t = 0; t < T; ++t) {
    dg_ += Qu_[t].transpose() * k_[t];
    dq_ -= k_[t].transpose() * Quuk_[t];
    if (!is_feasible_) {
      dg_ -= Vx_[t].transpose() * gaps_[t];
      fTVxx_p_.noalias() = Vxx_[t] * gaps_[t];
      dq_ += gaps_[t].transpose() * fTVxx_p_;
    }
  }
}

const Eigen::Vector2d& SolverFDDP::expectedImprovement() {
  dv_ = 0;
  d_.fill(0);
  unsigned int const& T = this->problem_.get_T();
  if (!is_feasible_) {
    problem_.running_models_.back()->get_state().diff(xs_try_.back(), xs_.back(), dx_.back());
    fTVxx_p_.noalias() = Vxx_.back() * dx_.back();
    dv_ -= gaps_.back().transpose() * fTVxx_p_;
    for (unsigned int t = 0; t < T; ++t) {
      problem_.running_models_[t]->get_state().diff(xs_try_[t], xs_[t], dx_[t]);
      fTVxx_p_.noalias() = Vxx_[t] * dx_[t];
      dv_ -= gaps_[t].transpose() * fTVxx_p_;
    }
  }
  d_[0] = dg_ + dv_;
  d_[1] = dq_ - 2 * dv_;
  return d_;
}

double SolverFDDP::calc() {
  cost_ = problem_.calcDiff(xs_, us_);
  if (!is_feasible_) {
    const Eigen::VectorXd& x0 = problem_.get_x0();
    problem_.running_models_[0]->get_state().diff(xs_[0], x0, gaps_[0]);

    unsigned int const& T = problem_.get_T();
    for (unsigned int t = 0; t < T; ++t) {
      ActionModelAbstract* model = problem_.running_models_[t];
      boost::shared_ptr<ActionDataAbstract>& d = problem_.running_datas_[t];
      model->get_state().diff(xs_[t + 1], d->get_xnext(), gaps_[t + 1]);
    }
  } else if (!was_feasible_) {
    for (std::vector<Eigen::VectorXd>::iterator it = gaps_.begin(); it != gaps_.end(); ++it) {
      it->setZero();
    }
  }
  return cost_;
}

void SolverFDDP::backwardPass() {
  boost::shared_ptr<ActionDataAbstract>& d_T = problem_.terminal_data_;
  Vxx_.back() = d_T->get_Lxx();
  Vx_.back() = d_T->get_Lx();

  x_reg_.fill(xreg_);
  if (!std::isnan(xreg_)) {
    Vxx_.back().diagonal() += x_reg_;
  }

  if (!is_feasible_) {
    Vx_.back() += Vxx_.back() * gaps_.back();
  }

  for (int t = static_cast<int>(problem_.get_T()) - 1; t >= 0; --t) {
    ActionModelAbstract* m = problem_.running_models_[t];
    boost::shared_ptr<ActionDataAbstract>& d = problem_.running_datas_[t];
    const Eigen::MatrixXd& Vxx_p = Vxx_[t + 1];
    const Eigen::VectorXd& Vx_p = Vx_[t + 1];

    FxTVxx_p_.noalias() = d->get_Fx().transpose() * Vxx_p;
    FuTVxx_p_[t].noalias() = d->get_Fu().transpose() * Vxx_p;
    Qxx_[t].noalias() = d->get_Lxx() + FxTVxx_p_ * d->get_Fx();
    Qxu_[t].noalias() = d->get_Lxu() + FxTVxx_p_ * d->get_Fu();
    Quu_[t].noalias() = d->get_Luu() + FuTVxx_p_[t] * d->get_Fu();
    Qx_[t].noalias() = d->get_Lx() + d->get_Fx().transpose() * Vx_p;
    Qu_[t].noalias() = d->get_Lu() + d->get_Fu().transpose() * Vx_p;

    if (!std::isnan(ureg_)) {
      unsigned int const& nu = m->get_nu();
      Quu_[t].diagonal() += Eigen::VectorXd::Constant(nu, ureg_);
    }

    computeGains(t);

    if (std::isnan(ureg_)) {
      Vx_[t].noalias() = Qx_[t] - K_[t].transpose() * Qu_[t];
    } else {
      Quuk_[t].noalias() = Quu_[t] * k_[t];
      Vx_[t].noalias() = Qx_[t] + K_[t].transpose() * Quuk_[t] - 2 * K_[t].transpose() * Qu_[t];
    }
    Vxx_[t].noalias() = Qxx_[t] - Qxu_[t] * K_[t];
    Vxx_[t] = 0.5 * (Vxx_[t] + Vxx_[t].transpose()).eval();
    // TODO(cmastalli): as suggested by Nicolas

    if (!std::isnan(xreg_)) {
      Vxx_[t].diagonal() += x_reg_;
    }

    // Compute and store the Vx gradient at end of the interval (rollout state)
    if (!is_feasible_) {
      Vx_[t].noalias() += Vxx_[t] * gaps_[t];
    }

    if (raiseIfNaN(Vx_[t].lpNorm<Eigen::Infinity>())) {
      throw "backward_error";
    }
    if (raiseIfNaN(Vxx_[t].lpNorm<Eigen::Infinity>())) {
      throw "backward_error";
    }
  }
}

void SolverFDDP::forwardPass(const double& steplength) {
  assert(steplength <= 1. && "Step length has to be <= 1.");
  assert(steplength >= 0. && "Step length has to be >= 0.");
  cost_try_ = 0.;
  xnext_ = problem_.get_x0();
  unsigned int const& T = problem_.get_T();
  for (unsigned int t = 0; t < T; ++t) {
    ActionModelAbstract* m = problem_.running_models_[t];
    boost::shared_ptr<ActionDataAbstract>& d = problem_.running_datas_[t];
    if ((is_feasible_) || (steplength == 1)) {
      xs_try_[t] = xnext_;
    } else {
      m->get_state().integrate(xnext_, gaps_[t] * (steplength - 1), xs_try_[t]);
    }
    m->get_state().diff(xs_[t], xs_try_[t], dx_[t]);
    us_try_[t].noalias() = us_[t] - k_[t] * steplength - K_[t] * dx_[t];
    m->calc(d, xs_try_[t], us_try_[t]);
    cost_try_ += d->cost;

    if (raiseIfNaN(cost_try_)) {
      throw "forward_error";
    }
    if (raiseIfNaN(d->get_xnext().lpNorm<Eigen::Infinity>())) {
      throw "forward_error";
    }
  }

  ActionModelAbstract* m = problem_.terminal_model_;
  boost::shared_ptr<ActionDataAbstract>& d = problem_.terminal_data_;

  if ((is_feasible_) || (steplength == 1)) {
    xs_try_.back() = problem_.running_datas_.back()->get_xnext();
  } else {
    m->get_state().integrate(problem_.running_datas_.back()->get_xnext(), gaps_.back() * (steplength - 1),
                             xs_try_.back());
  }
  m->calc(d, xs_try_.back());
  cost_try_ += d->cost;

  if (raiseIfNaN(cost_try_)) {
    throw "forward_error";
  }
}

}  // namespace crocoddyl