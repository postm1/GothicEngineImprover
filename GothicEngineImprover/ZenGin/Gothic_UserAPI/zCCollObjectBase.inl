// Supported with union (c) 2020 Union team

// User API for zCCollObjectBase
// Add your methods here

zCVob* GetVob() const { return static_cast<zCVob*>(this->m_pvClientData); };