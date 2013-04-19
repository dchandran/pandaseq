/* PANDAseq -- Assemble paired FASTQ Illumina reads and strip the region between amplification primers.
     Copyright (C) 2011-2012  Andre Masella

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */
#include "config.h"
#include <math.h>
#include "pandaseq.h"
#include "nt.h"
#include "prob.h"
#include "table.h"

static char ntchar[16] = { 'N', 'A', 'C', 'M', 'G', 'R', 'S', 'V', 'T', 'W', 'Y', 'H', 'K', 'D', 'B', 'N' };

panda_nt iupac_forward[32] = { /* @ */ (panda_nt) 0, /*A*/ PANDA_NT_A,
	 /*B*/ PANDA_NT_C | PANDA_NT_G | PANDA_NT_T,
	 /*C*/ PANDA_NT_C,
	 /*D*/ PANDA_NT_A | PANDA_NT_G | PANDA_NT_T, /*E*/(panda_nt) 0, /*F*/(
		panda_nt) 0,
	 /*G*/ PANDA_NT_G,
	 /*H*/ PANDA_NT_A | PANDA_NT_C | PANDA_NT_T,
	 /*I*/(
		panda_nt) 0, /*J*/(
		panda_nt) 0, /*K*/ PANDA_NT_G | PANDA_NT_T, /*L*/(
		panda_nt) 0,
	 /*M*/ PANDA_NT_A | PANDA_NT_C,
	 /*N*/ PANDA_NT_A | PANDA_NT_C | PANDA_NT_G | PANDA_NT_T, /*O*/(
		panda_nt) 0,
	 /*P*/(
		panda_nt) 0, /*Q*/(
		panda_nt) 0,
	 /*R*/ PANDA_NT_A | PANDA_NT_G,
	 /*S*/ PANDA_NT_C | PANDA_NT_G, /*T*/ PANDA_NT_T, /*U*/ PANDA_NT_T,
	 /*V*/ PANDA_NT_A | PANDA_NT_C | PANDA_NT_G,
	 /*W*/ PANDA_NT_A | PANDA_NT_T,
	 /*X*/ PANDA_NT_A | PANDA_NT_C | PANDA_NT_G | PANDA_NT_T, /*Y*/ PANDA_NT_C | PANDA_NT_T, /*Z*/(
		panda_nt) 0, /*[ */ (
		panda_nt) 0, /*\ */ (
		panda_nt) 0,	/*] */
	(
		panda_nt) 0, /*^ */ (
		panda_nt) 0, /*_*/ (
		panda_nt) 0
};

panda_nt iupac_reverse[32] = { /* @ */ (panda_nt) 0, /*A*/ PANDA_NT_T,
	 /*B*/ PANDA_NT_G | PANDA_NT_C | PANDA_NT_A,
	 /*C*/ PANDA_NT_G,
	 /*D*/ PANDA_NT_T | PANDA_NT_C | PANDA_NT_A, /*E*/(panda_nt) 0, /*F*/(
		panda_nt) 0,
	 /*G*/ PANDA_NT_C,
	 /*H*/ PANDA_NT_T | PANDA_NT_G | PANDA_NT_A,
	 /*I*/(
		panda_nt) 0, /*J*/(
		panda_nt) 0, /*K*/ PANDA_NT_C | PANDA_NT_A, /*L*/(
		panda_nt) 0,
	 /*M*/ PANDA_NT_T | PANDA_NT_G,
	 /*N*/ PANDA_NT_A | PANDA_NT_C | PANDA_NT_G | PANDA_NT_T, /*O*/(
		panda_nt) 0,
	 /*P*/(
		panda_nt) 0, /*Q*/(
		panda_nt) 0,
	 /*R*/ PANDA_NT_T | PANDA_NT_C,
	 /*S*/ PANDA_NT_G | PANDA_NT_C, /*T*/ PANDA_NT_A, /*U*/ PANDA_NT_A,
	 /*V*/ PANDA_NT_T | PANDA_NT_G | PANDA_NT_C,
	 /*W*/ PANDA_NT_T | PANDA_NT_A,
	 /*X*/ PANDA_NT_A | PANDA_NT_C | PANDA_NT_G | PANDA_NT_T, /*Y*/ PANDA_NT_G | PANDA_NT_A, /*Z*/(
		panda_nt) 0, /*[ */ (
		panda_nt) 0, /*\ */ (
		panda_nt) 0,	/*] */
	(
		panda_nt) 0, /*^ */ (
		panda_nt) 0, /*_*/ (
		panda_nt) 0
};

double
panda_quality_probability(
	const panda_qual *q) {
	return exp(panda_quality_log_probability(q));
}

double
panda_quality_log_probability(
	const panda_qual *q) {
	int index = (int) q->qual;
	if (index < 0) {
		index = 0;
	} else if (index > PHREDMAX) {
		index = PHREDMAX;
	}
	return qual_score[index];
}

panda_nt panda_nt_from_ascii(
	char c) {
	return iupac_forward[(int) c & 0x1F];
}

panda_nt panda_nt_from_ascii_complement(
	char c) {
	return iupac_reverse[(int) c & 0x1F];
}

char panda_nt_to_ascii(
	panda_nt val) {
	if (val < (panda_nt) 0 || val > (panda_nt) 15) {
		return 'N';
	}
	return ntchar[(int) (val)];
}
