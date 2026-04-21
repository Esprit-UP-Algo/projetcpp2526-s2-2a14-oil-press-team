# Finalizing Personnel Automation: Capacity Planner & Request Hub

This plan outlines the steps required to remove empty "shells" and fully implement the mocked functionality within both the `Capacity Planner` and the `Request Hub`, ensuring a seamless shift-management loop.

## User Review Required

> [!IMPORTANT]
> Please review the proposed changes below. Once approved, I will implement them so your AI scheduler and leave management systems are fully operational.

## Proposed Changes

---

### Capacity Planner Updates

#### [MODIFY] `workforceplanner.cpp` & `workforceplanner.h`

**1. Live KPI Statistics (`refreshKpis`)**
- Calculate real metrics instead of `"Calculating..."`.
- **Coverage Gaps**: Query the database to find how many shifts this week fall below the optimal staffing count (5 per shift).
- **Overscheduled**: Calculate shifts that have an unusually high number of operators assigned.
- **Capacity Score**: Mathematically compute the percentage of fulfilled minimum shift requirements for the current week instead of hardcoding `85%`.

**2. Shift Assignments Visualizer (`refreshAssignments`)**
- Replace the `"Assignments loaded... (Formatting hidden for brevity)"` label with a functional `QTableWidget`.
- The table will list the Shift Type, Day of Week, Employee Name, Role, and Grade for an easily readable roster of who works when.

**3. Dynamic Heatmap Logic (`refreshHeatmap`)**
- Replace hardcoded thresholds (`0` for Red, `< 3` for Amber) with values driven by the `ScheduleConfig`.
- Optimal shift size = `config.minPressOperator + config.minMaintenance + config.minQualityControl + config.minLogistics` (Total of 5).
- Heatmap rules will become: 0 = Critical, < 5 = Understaffed, >= 5 = Optimal.

---

### Request Hub Updates

#### [MODIFY] `mainwindow.cpp`

**1. "Submit Leave Request" Form**
- Add a new "Submit Leave" button to the Request Hub header next to the "Refresh" button.
- Build a popup `QDialog` containing inputs for:
  - **Employee CIN** (Dropdown or text)
  - **Start Date** (`QDateEdit`)
  - **End Date** (`QDateEdit`) *(Utilizing the `LEAVE_END` column you confirmed exists!)*
- Submitting this form will `INSERT` a row into `SHIFT_SCHEDULE` with the status `Pending` (or `NULL` as your schema defines it), successfully feeding the Request Hub queue.

**2. Improved Request Table Rendering**
- Update the SQL query in `refreshRequests()` to fetch and display the `LEAVE_END` date alongside the `LEAVE_START` date to represent the full duration of a vacation or absence.

## Open Questions

- Should an employee automatically be *removed* from any shifts they were previously scheduled for if a manager hits "Approve" on a leave request overlapping those dates? I can enforce a `DELETE FROM SHIFT_SCHEDULE` trigger in the GUI when approval is clicked.

## Verification Plan

### Manual Verification
1. I will navigate to the Capacity Planner and verify KPIs populate real mathematical outputs.
2. I will verify the Shift Assignments tab displays a generated table of staff.
3. I will open the Request Hub, use the new "Submit Leave" button, verify a Pending row appears, and hit Approve/Reject to ensure it interacts correctly with the Oracle database.
