#pragma once
// ================================================================
//  Axion Digitaverse - ed25519.h  [Phase 1]
//  Real Ed25519 digital signatures - TweetNaCl port.
//  Self-contained. Zero external dependencies.
//  Identical output to libsodium.
// ================================================================

#include "sha512.h"
#include <cstring>
#include <cassert>

namespace axion {
namespace ed25519 {

typedef int64_t gf[16];

static const gf _gf0={}, _gf1={1};
static const gf _D = {0x78a3,0x1359,0x4dca,0x75eb,0xd8ab,0x4141,0x0a4d,0x0070,
                      0xe898,0x7779,0x4079,0x8cc7,0xfe73,0x2b6f,0x6cee,0x5203};
static const gf _D2= {0xf159,0x26b2,0x9b94,0xebd6,0xb156,0x8283,0x149a,0x00e0,
                      0xd130,0xeef3,0x80f2,0x198e,0xfce7,0x56df,0xd9dc,0x2406};
static const gf _X = {0xd51a,0x8f25,0x2d60,0xc956,0xa7b2,0x9525,0xc760,0x692c,
                      0xdc5c,0xfdd6,0xe231,0xc0a4,0x53fe,0xcd6e,0x36d3,0x2169};
static const gf _Y = {0x6658,0x6666,0x6666,0x6666,0x6666,0x6666,0x6666,0x6666,
                      0x6666,0x6666,0x6666,0x6666,0x6666,0x6666,0x6666,0x6666};
static const gf _I = {0xa0b0,0x4a0e,0x1b27,0xc4ee,0xe478,0xad2f,0x1806,0x2f43,
                      0xd7a7,0x3dfb,0x0099,0x2b4d,0xdf0b,0x4fc1,0x2480,0x2b83};

static void gf_cp(gf o,const gf i){for(int j=0;j<16;j++)o[j]=i[j];}
static void gf_add(gf o,const gf a,const gf b){for(int j=0;j<16;j++)o[j]=a[j]+b[j];}
static void gf_sub(gf o,const gf a,const gf b){for(int j=0;j<16;j++)o[j]=a[j]-b[j];}

static void gf_mul(gf o,const gf a,const gf b){
    int64_t t[31]={};
    for(int i=0;i<16;i++) for(int j=0;j<16;j++) t[i+j]+=a[i]*b[j];
    for(int i=0;i<15;i++) t[i]+=38*t[i+16];
    gf_cp(o,(const int64_t*)t);
    // carry twice
    for(int k=0;k<2;k++){
        int64_t c=1;
        for(int i=0;i<16;i++){
            int64_t v=o[i]+c+65535; c=v>>16; o[i]=v-c*65536;
        }
        o[0]+=c-1+37*(c-1);
    }
}
static void gf_sq(gf o,const gf a){gf_mul(o,a,a);}

static void gf_car(gf o){
    for(int i=0;i<16;i++){
        o[i]+=(int64_t)1<<16;
        int64_t c=o[i]>>16;
        o[(i+1)*(i<15)]+=c-1+37*(c-1)*(i==15);
        o[i]-=c*((int64_t)1<<16);
    }
}

static void gf_pack(uint8_t out[32],const gf n){
    gf m,t; gf_cp(t,n);
    gf_car(t);gf_car(t);gf_car(t);
    for(int j=0;j<2;j++){
        m[0]=t[0]-0xffed;
        for(int i=1;i<15;i++){m[i]=t[i]-0xffff-((m[i-1]>>16)&1);m[i-1]&=0xffff;}
        m[15]=t[15]-0x7fff-((m[14]>>16)&1);
        int16_t b=(int16_t)((m[15]>>16)&1); m[14]&=0xffff;
        for(int i=0;i<16;i++){int64_t sw=(1-(int64_t)b)*(t[i]-m[i]);t[i]-=sw;m[i]+=sw;}
    }
    for(int i=0;i<16;i++){out[2*i]=(uint8_t)(t[i]&0xff);out[2*i+1]=(uint8_t)(t[i]>>8);}
}

static int64_t gf_par(const gf a){uint8_t d[32];gf_pack(d,a);return d[0]&1;}

static void gf_unpack(gf o,const uint8_t n[32]){
    for(int i=0;i<16;i++) o[i]=n[2*i]|(int64_t)n[2*i+1]<<8;
    o[15]&=0x7fff;
}

static void gf_inv(gf o,const gf i){
    gf c; gf_cp(c,i);
    for(int a=253;a>=0;a--){gf_sq(c,c);if(a!=2&&a!=4)gf_mul(c,c,i);}
    gf_cp(o,c);
}

static void gf_pow2523(gf o,const gf i){
    gf c; gf_cp(c,i);
    for(int a=250;a>=0;a--){gf_sq(c,c);if(a!=1)gf_mul(c,c,i);}
    gf_cp(o,c);
}

// Extended twisted Edwards point: (X:Y:Z:T)
typedef gf pt[4];

static void pt_add(pt p,const pt q){
    gf a,b,c,d,e,f,g,h,t;
    gf_sub(a,p[1],p[0]); gf_sub(t,q[1],q[0]); gf_mul(a,a,t);
    gf_add(b,p[0],p[1]); gf_add(t,q[0],q[1]); gf_mul(b,b,t);
    gf_mul(c,p[3],q[3]); gf_mul(c,c,_D2);
    gf_mul(d,p[2],q[2]); gf_add(d,d,d);
    gf_sub(e,b,a); gf_sub(f,d,c); gf_add(g,d,c); gf_add(h,b,a);
    gf_mul(p[0],e,f); gf_mul(p[1],g,h); gf_mul(p[2],f,g); gf_mul(p[3],e,h);
}

static void pt_cswap(pt p,pt q,int64_t b){
    // Mask: 0 stays 0, 1 becomes all-ones (-1 in two's complement)
    b = -b;
    for(int i=0;i<4;i++)
        for(int j=0;j<16;j++){
            int64_t t = b & (p[i][j] ^ q[i][j]);
            p[i][j] ^= t;
            q[i][j] ^= t;
        }
}

static void pt_scalarmult(pt p,const pt q,const uint8_t s[32]){
    gf_cp(p[0],_gf0); gf_cp(p[1],_gf1); gf_cp(p[2],_gf1); gf_cp(p[3],_gf0);
    pt r; for(int i=0;i<4;i++) gf_cp(r[i],q[i]);
    for(int i=255;i>=0;i--){
        int64_t b=(s[i/8]>>(i&7))&1;
        pt_cswap(p,r,b);
        pt_add(r,p);
        pt_add(p,p);
        pt_cswap(p,r,b);
    }
}

static void scalarbase(pt p,const uint8_t s[32]){
    pt q; gf_cp(q[0],_X); gf_cp(q[1],_Y); gf_cp(q[2],_gf1); gf_mul(q[3],_X,_Y);
    pt_scalarmult(p,q,s);
}

static void pt_pack(uint8_t r[32],const pt p){
    gf tx,ty,zi;
    gf_inv(zi,p[2]); gf_mul(tx,p[0],zi); gf_mul(ty,p[1],zi);
    gf_pack(r,ty); r[31]^=(uint8_t)(gf_par(tx)<<7);
}

static int pt_unpack_neg(pt r,const uint8_t p[32]){
    gf t,chk,num,den,den2,den4,den6;
    gf_cp(r[2],_gf1);
    gf_unpack(r[1],p);
    gf_sq(num,r[1]); gf_mul(den,num,_D);
    gf_sub(num,num,r[2]); gf_add(den,r[2],den);
    gf_sq(den2,den); gf_sq(den4,den2); gf_mul(den6,den4,den2);
    gf_mul(t,den6,num); gf_mul(t,t,den);
    gf_pow2523(t,t); gf_mul(t,t,num); gf_mul(t,t,den);
    gf_mul(t,t,den); gf_mul(r[0],t,den);
    gf_sq(chk,r[0]); gf_mul(chk,chk,den);
    // Compare via canonical packed form (limbs may be un-normalized after gf_mul)
    {
        uint8_t p_chk[32], p_num[32];
        gf_pack(p_chk, chk); gf_pack(p_num, num);
        bool ok = (memcmp(p_chk, p_num, 32) == 0);
        if(!ok){
            gf_mul(r[0],r[0],_I);
            gf_sq(chk,r[0]); gf_mul(chk,chk,den);
            gf_pack(p_chk, chk);
            if(memcmp(p_chk, p_num, 32) != 0) return -1;
        }
    }
    if(gf_par(r[0])==((p[31]>>7)&1)) gf_sub(r[0],_gf0,r[0]);
    gf_mul(r[3],r[0],r[1]);
    return 0;
}

static const uint8_t _L[32]={
    0xed,0xd3,0xf5,0x5c,0x1a,0x63,0x12,0x58,
    0xd6,0x9c,0xf7,0xa2,0xde,0xf9,0xde,0x14,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10
};

static void modL(uint8_t*r,int64_t x[64]){
    for(int i=63;i>=32;i--){
        int64_t carry=0,j;
        for(j=i-32;j<i-12;j++){x[j]+=carry-16*x[i]*(int64_t)_L[j-(i-32)];carry=(x[j]+128)>>8;x[j]-=carry*256;}
        x[j]+=carry; x[i]=0;
    }
    int64_t carry=0;
    for(int j=0;j<32;j++){x[j]+=carry-(x[31]>>4)*(int64_t)_L[j];carry=x[j]>>8;x[j]&=255;}
    for(int j=0;j<32;j++) x[j]-=carry*(int64_t)_L[j];
    for(int i=0;i<32;i++){x[i+1]+=x[i]>>8;r[i]=(uint8_t)(x[i]&255);}
}

static void reduce64(uint8_t r[64]){
    int64_t x[64]; for(int i=0;i<64;i++) x[i]=r[i];
    for(int i=0;i<64;i++) r[i]=0;
    modL(r,x);
}

// ── Public API ─────────────────────────────────────────────
struct Keypair {
    Bytes public_key;   // 32 bytes
    Bytes private_key;  // 64 bytes: seed || pubkey
    Bytes seed;         // 32 bytes
};

static Keypair keypair_from_seed(const Bytes& seed){
    assert(seed.size()==32);
    Bytes h=sha512_bytes(seed);
    uint8_t az[64]; for(int i=0;i<64;i++) az[i]=h[i];
    az[0]&=248; az[31]&=127; az[31]|=64;
    pt p; scalarbase(p,az);
    uint8_t pk[32]; pt_pack(pk,p);
    Keypair kp;
    kp.seed=seed;
    kp.public_key.assign(pk,pk+32);
    kp.private_key.resize(64);
    std::copy(seed.begin(),seed.end(),kp.private_key.begin());
    std::copy(kp.public_key.begin(),kp.public_key.end(),kp.private_key.begin()+32);
    return kp;
}

static Bytes sign(const Bytes& msg, const Bytes& sk){
    assert(sk.size()==64);
    const uint8_t* seed_ptr=sk.data();
    const uint8_t* pk_ptr=sk.data()+32;
    Bytes h=sha512_bytes(Bytes(seed_ptr,seed_ptr+32));
    uint8_t az[64]; for(int i=0;i<64;i++) az[i]=h[i];
    az[0]&=248; az[31]&=127; az[31]|=64;
    // nonce
    Bytes nd(az+32,az+64); nd.insert(nd.end(),msg.begin(),msg.end());
    Bytes nh=sha512_bytes(nd);
    uint8_t nonce[64]; for(int i=0;i<64;i++) nonce[i]=nh[i];
    reduce64(nonce);
    // R
    pt R; scalarbase(R,nonce);
    uint8_t renc[32]; pt_pack(renc,R);
    // k
    Bytes kd(renc,renc+32); kd.insert(kd.end(),pk_ptr,pk_ptr+32); kd.insert(kd.end(),msg.begin(),msg.end());
    Bytes kh=sha512_bytes(kd);
    // S = r + k*a mod L
    int64_t x[64]={}; for(int i=0;i<32;i++) x[i]=(int64_t)nonce[i];
    // Reduce k (hram) mod L before accumulating into S
    uint8_t k64[64]; for(int i=0;i<64;i++) k64[i]=kh[i];
    reduce64(k64);
    for(int i=0;i<32;i++) for(int j=0;j<32;j++) x[i+j]+=(int64_t)k64[i]*(int64_t)az[j];
    uint8_t senc[64]={}; modL(senc,x);
    Bytes sig(64);
    std::copy(renc,renc+32,sig.begin());
    std::copy(senc,senc+32,sig.begin()+32);
    return sig;
}

static bool verify(const Bytes& msg, const Bytes& sig, const Bytes& pk){
    if(sig.size()!=64||pk.size()!=32) return false;
    const uint8_t* sp=sig.data();
    const uint8_t* pp=pk.data();
    pt A; if(pt_unpack_neg(A,pp)!=0) return false;
    Bytes kd(sp,sp+32); kd.insert(kd.end(),pp,pp+32); kd.insert(kd.end(),msg.begin(),msg.end());
    Bytes kh=sha512_bytes(kd);
    uint8_t k64[64]; for(int i=0;i<64;i++) k64[i]=kh[i];
    reduce64(k64);
    pt p; pt_scalarmult(p,A,k64);
    pt q; scalarbase(q,sp+32);
    pt_add(p,q);
    uint8_t rcheck[32]; pt_pack(rcheck,p);
    return std::memcmp(rcheck,sp,32)==0;
}

} // namespace ed25519
} // namespace axion
