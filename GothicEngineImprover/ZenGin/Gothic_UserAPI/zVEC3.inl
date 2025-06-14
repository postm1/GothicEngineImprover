// Supported with union (c) 2020 Union team

// User API for zVEC3
// Add your methods here

zSTRING ToString() const {
	return "(" + zSTRING(n[0], 10) + ", " + zSTRING(n[1], 10) + ", " + zSTRING(n[2], 10) + ")";
}