let
	pkgs = import <nixpkgs> { };
in {
	default = pkgs.callPackage (

{ stdenv
, lib
, cmake
, pkg-config
, libogg
, libvorbis
, flac
, libsndfile
, libopus
}:

stdenv.mkDerivation rec {
	pname = "pxtone-renderer";
	version = "local";

	src = lib.cleanSource ./.;

	nativeBuildInputs = [
		cmake
		pkg-config
	];

	buildInputs = [
		libogg
		libvorbis
		flac
		(libsndfile.overrideAttrs (oa: {
			nativeBuildInputs = oa.nativeBuildInputs ++ [
				cmake
			];
			propagatedBuildInputs = [
				libopus
			];
		}))
	];

	cmakeBuildType = "Debug";
	dontStrip = true;

	installPhase = ''
		runHook preInstall

		install -Dm755 {.,$out/bin}/pxtone-renderer

		runHook postInstall
	'';
}

	) {};
}
